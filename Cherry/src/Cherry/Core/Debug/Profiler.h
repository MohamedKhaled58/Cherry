#pragma once
#include "imgui.h"

#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

namespace Cherry {

    // Forward declaration for ImGui
    struct ImVec4;

    struct ProfileData
    {
        std::string Name;
        float Duration = 0.0f;           // Current frame duration in ms
        float AverageDuration = 0.0f;    // Rolling average duration in ms
        float MinDuration = FLT_MAX;     // Minimum duration recorded
        float MaxDuration = 0.0f;        // Maximum duration recorded
        uint32_t CallCount = 0;          // Number of calls this frame
        uint32_t TotalCalls = 0;         // Total calls since start
        std::vector<float> History;      // Duration history for graphing
        bool IsActive = false;           // Currently being profiled

        static constexpr size_t MAX_HISTORY = 100;

        void AddSample(float duration)
        {
            Duration = duration;
            MinDuration = std::min(MinDuration, duration);
            MaxDuration = std::max(MaxDuration, duration);
            CallCount++;
            TotalCalls++;

            // Add to history for graphing
            History.push_back(duration);
            if (History.size() > MAX_HISTORY)
                History.erase(History.begin());

            // Calculate rolling average
            if (History.size() > 0)
            {
                float sum = 0.0f;
                for (float h : History)
                    sum += h;
                AverageDuration = sum / History.size();
            }
        }

        void Reset()
        {
            CallCount = 0;
            IsActive = false;
        }
    };

    class Profiler
    {
    private:
        std::unordered_map<std::string, ProfileData> m_ProfileData;
        std::mutex m_DataMutex;
        bool m_IsEnabled = true;
        float m_FrameTime = 0.0f;
        std::vector<float> m_FrameHistory;
        static constexpr size_t MAX_FRAME_HISTORY = 120;

    public:
        static Profiler& Get()
        {
            static Profiler instance;
            return instance;
        }

        void SetEnabled(bool enabled) { m_IsEnabled = enabled; }
        bool IsEnabled() const { return m_IsEnabled; }

        void BeginProfile(const std::string& name)
        {
            if (!m_IsEnabled) return;

            std::lock_guard<std::mutex> lock(m_DataMutex);
            m_ProfileData[name].IsActive = true;
        }

        void EndProfile(const std::string& name, float duration)
        {
            if (!m_IsEnabled) return;

            std::lock_guard<std::mutex> lock(m_DataMutex);
            auto& data = m_ProfileData[name];
            data.AddSample(duration);
            data.IsActive = false;
        }

        void SetFrameTime(float frameTime)
        {
            m_FrameTime = frameTime;
            m_FrameHistory.push_back(frameTime);
            if (m_FrameHistory.size() > MAX_FRAME_HISTORY)
                m_FrameHistory.erase(m_FrameHistory.begin());
        }

        void NewFrame()
        {
            if (!m_IsEnabled) return;

            std::lock_guard<std::mutex> lock(m_DataMutex);
            for (auto& [name, data] : m_ProfileData)
            {
                data.Reset();
            }
        }

        // ImGui rendering function
        void OnImGuiRender()
        {
            if (!m_IsEnabled) return;

            std::lock_guard<std::mutex> lock(m_DataMutex);

            if (ImGui::Begin("Performance Profiler"))
            {
                // Enable/Disable profiling
                ImGui::Checkbox("Enable Profiling", &m_IsEnabled);
                ImGui::Separator();

                // Frame time display
                ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", m_FrameTime, 1000.0f / m_FrameTime);

                // Frame time graph
                if (!m_FrameHistory.empty())
                {
                    ImGui::PlotLines("Frame Time History", m_FrameHistory.data(),
                        (int)m_FrameHistory.size(), 0, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
                }

                ImGui::Separator();

                // Profile data table
                if (ImGui::BeginTable("ProfilerTable", 6, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
                {
                    ImGui::TableSetupColumn("Function");
                    ImGui::TableSetupColumn("Current (ms)");
                    ImGui::TableSetupColumn("Average (ms)");
                    ImGui::TableSetupColumn("Min (ms)");
                    ImGui::TableSetupColumn("Max (ms)");
                    ImGui::TableSetupColumn("Calls");
                    ImGui::TableHeadersRow();

                    // Sort by current duration (descending)
                    std::vector<std::pair<std::string, ProfileData*>> sortedData;
                    for (auto& [name, data] : m_ProfileData)
                    {
                        sortedData.emplace_back(name, &data);
                    }

                    std::sort(sortedData.begin(), sortedData.end(),
                        [](const auto& a, const auto& b) {
                            return a.second->Duration > b.second->Duration;
                        });

                    for (const auto& [name, data] : sortedData)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();

                        // Color code active functions
                        if (data->IsActive)
                            ImGui::TextColored(::ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", name.c_str());
                        else
                            ImGui::Text("%s", name.c_str());

                        ImGui::TableNextColumn();
                        if (data->Duration > 16.67f) // Red if over 60fps budget
                            ImGui::TextColored(::ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%.3f", data->Duration);
                        else if (data->Duration > 8.33f) // Yellow if over 120fps budget
                            ImGui::TextColored(::ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.3f", data->Duration);
                        else
                            ImGui::Text("%.3f", data->Duration);

                        ImGui::TableNextColumn();
                        ImGui::Text("%.3f", data->AverageDuration);

                        ImGui::TableNextColumn();
                        ImGui::Text("%.3f", data->MinDuration == FLT_MAX ? 0.0f : data->MinDuration);

                        ImGui::TableNextColumn();
                        ImGui::Text("%.3f", data->MaxDuration);

                        ImGui::TableNextColumn();
                        ImGui::Text("%u", data->CallCount);

                        // Show mini graph on right-click
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !data->History.empty())
                        {
                            ImGui::OpenPopup(("Graph##" + name).c_str());
                        }

                        if (ImGui::BeginPopup(("Graph##" + name).c_str()))
                        {
                            ImGui::Text("%s Performance History", name.c_str());
                            ImGui::PlotLines("", data->History.data(), (int)data->History.size(),
                                0, nullptr, 0.0f, data->MaxDuration * 1.1f, ImVec2(300, 100));
                            ImGui::EndPopup();
                        }
                    }

                    ImGui::EndTable();
                }

                // Clear all data button
                ImGui::Separator();
                if (ImGui::Button("Clear All Data"))
                {
                    m_ProfileData.clear();
                    m_FrameHistory.clear();
                }
            }
            ImGui::End();
        }

        // Get profile data for custom displays
        const std::unordered_map<std::string, ProfileData>& GetProfileData() const
        {
            return m_ProfileData;
        }
    };

    class ScopedTimer
    {
    public:
        ScopedTimer(const std::string& name)
            : m_Name(name)
        {
            Profiler::Get().BeginProfile(m_Name);
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        ~ScopedTimer()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime);
            float ms = duration.count() * 0.001f; // Convert to milliseconds
            Profiler::Get().EndProfile(m_Name, ms);
        }

    private:
        std::string m_Name;
        std::chrono::high_resolution_clock::time_point m_StartTime;
    };

    // Helper class for frame timing
    class FrameTimer
    {
    private:
        std::chrono::high_resolution_clock::time_point m_LastFrameTime;

    public:
        FrameTimer()
        {
            m_LastFrameTime = std::chrono::high_resolution_clock::now();
        }

        void NewFrame()
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_LastFrameTime);
            float frameTime = duration.count() * 0.001f; // Convert to milliseconds

            Profiler::Get().SetFrameTime(frameTime);
            Profiler::Get().NewFrame();

            m_LastFrameTime = currentTime;
        }
    };
}