#pragma once
#include <Framework/FrameWork.hpp>
#include "Utils.hpp"
#include <chrono>
#include <map>
#include "Globals.hpp"

namespace NotifyManager
{
    enum eType {
        None,
        Info,
        Warning
    };

    enum eState {
        In,
        Current,
        Out,
        Expired
    };

    class NotifyClass {
    private:
        std::string Title;
        std::string Description;
        time_t ExpireTime = 0;
        time_t CreationTime = 0;
        eType Type = eType::None;
        eState CurrentState = eState::In;
    public:
        void SetTitle(std::string NewTitle) { this->Title = NewTitle; }
        void SetDescription(std::string NewDesc) { this->Description = NewDesc; }
        void SetType(eType NewType) { this->Type = NewType; }
        void SetState(eState NewState) { this->CurrentState = NewState; }
        void SetCreationTime(time_t NewCreationTime) { this->CreationTime = NewCreationTime; }
    public:
        std::string GetTitle() { return Title; }
        std::string GetDescription() { return Description; }
        time_t GetExpireTime() { return ExpireTime; }
        time_t GetCreationTime() { return CreationTime; }
        eType GetType() { return Type; }
        eState GetCurrentState() { return CurrentState; }
    public:
        time_t GetNowTime() {
            using namespace std::chrono;
            return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        }

        time_t GetTimeDiff() {
            return (time_t)(GetNowTime() - CreationTime);
        }

        NotifyClass(eType Type, time_t ExpireTime = 4000)
        {
            this->Type = Type;
            this->ExpireTime = ExpireTime;
            this->CreationTime = GetNowTime();
            this->CurrentState = eState::In; // Garantir estado inicial
        }
    };

    inline std::vector<NotifyClass> NotifyList;

    inline void DeleteNotify(int Index)
    {
        if (Index >= 0 && Index < NotifyList.size()) {
            NotifyList.erase(NotifyList.begin() + Index);
        }
    }

    inline void Render()
    {
        // VERIFICA��O DE SEGURAN�A
        if (!ImGui::GetCurrentContext()) return;

        const auto DrawList = ImGui::GetForegroundDrawList();

        // CORRE��O: Usar DisplaySize do ImGui em vez de g_vGameWindowSize
        const auto WindowSize = ImGui::GetIO().DisplaySize;

        // VERIFICA��O DE TAMANHO V�LIDO
        if (WindowSize.x <= 0 || WindowSize.y <= 0) return;

        float NextHeight = 0.f;

        for (int i = 0; i < NotifyList.size(); i++)
        {
            auto& Notify = NotifyList[i];

            struct NotifyWid_t {
                float SlideX = 0.f;
                float YPos = 0.f;
                float Alpha = 0.f;
                bool Initialized = false;
            };

            static std::map<std::string, NotifyWid_t> anim;
            const auto Id = Notify.GetDescription() + std::to_string(Notify.GetCreationTime());
            auto NotifyAnim = anim.find(Id);

            if (NotifyAnim == anim.end())
            {
                NotifyWid_t newAnim;
                newAnim.SlideX = 0.f;
                newAnim.YPos = 0.f;
                newAnim.Alpha = 0.f;
                newAnim.Initialized = true;
                anim.insert({ Id, newAnim });
                NotifyAnim = anim.find(Id);
            }

            // VERIFICAR SE ANIMA��O FOI INICIALIZADA
            if (!NotifyAnim->second.Initialized) {
                NotifyAnim->second.SlideX = 0.f;
                NotifyAnim->second.Alpha = 0.f;
                NotifyAnim->second.Initialized = true;
            }

            // VERIFICAR EXPIRA��O
            if (Notify.GetCurrentState() == eState::Expired)
            {
                DeleteNotify(i);
                anim.erase(Id);
                i--;
                continue;
            }

            // CALCULAR TAMANHO DO TEXTO
            auto TitleTxtSize = ImGui::CalcTextSize(Notify.GetTitle().c_str());
            auto DescTxtSize = ImGui::CalcTextSize(Notify.GetDescription().c_str());

            const float Padding = 8.f;

            // CALCULAR TAMANHO DA NOTIFICA��O
            float NotifyWidth = fmaxf(TitleTxtSize.x, DescTxtSize.x) + (Padding * 2.f);
            float NotifyHeight = TitleTxtSize.y + DescTxtSize.y + (Padding * 3.f);
            ImVec2 NotifySize(NotifyWidth, NotifyHeight);

            // M�QUINA DE ESTADO - ANIMA��ES
            float deltaTime = ImGui::GetIO().DeltaTime;

            switch (Notify.GetCurrentState())
            {
            case eState::In:
            {
                NotifyAnim->second.SlideX = ImLerp(NotifyAnim->second.SlideX, NotifySize.x, deltaTime * 10.f);
                NotifyAnim->second.Alpha = ImLerp(NotifyAnim->second.Alpha, 1.f, deltaTime * 8.f);

                if (NotifyAnim->second.SlideX >= NotifySize.x * 0.95f) {
                    Notify.SetState(eState::Current);
                }
                break;
            }

            case eState::Current:
            {
                // Manter vis�vel
                NotifyAnim->second.SlideX = NotifySize.x;
                NotifyAnim->second.Alpha = 1.f;

                // Verificar se expirou
                if (Notify.GetTimeDiff() >= Notify.GetExpireTime()) {
                    Notify.SetState(eState::Out);
                }
                break;
            }

            case eState::Out:
            {
                NotifyAnim->second.SlideX = ImLerp(NotifyAnim->second.SlideX, 0.f, deltaTime * 8.f);
                NotifyAnim->second.Alpha = ImLerp(NotifyAnim->second.Alpha, 0.f, deltaTime * 6.f);

                if (NotifyAnim->second.SlideX <= 5.f) {
                    Notify.SetState(eState::Expired);
                }
                break;
            }
            }

            // CALCULAR POSI��O - CORRE��O IMPORTANTE
            float NotifyStartX = WindowSize.x - NotifyAnim->second.SlideX;
            float NotifyStartY = WindowSize.y - NotifySize.y - Padding - NextHeight;

            ImRect NotifyPos(
                ImVec2(NotifyStartX, NotifyStartY),
                ImVec2(NotifyStartX + NotifySize.x, NotifyStartY + NotifySize.y)
            );

            // APLICAR ALPHA
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, NotifyAnim->second.Alpha);

            // FUNDO DA NOTIFICA��O
            DrawList->AddRectFilled(NotifyPos.Min, NotifyPos.Max,
                ImGui::GetColorU32(ImVec4(0.05f, 0.05f, 0.05f, 0.95f)), 6.0f);

            // BORDA
            DrawList->AddRect(NotifyPos.Min, NotifyPos.Max,
                ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 0.8f)), 6.0f, 0, 1.5f);

            // BARRA DE PROGRESSO
            if (Notify.GetCurrentState() == eState::Current) {
                float Progress = 1.0f - ((float)Notify.GetTimeDiff() / (float)Notify.GetExpireTime());
                ImVec2 ProgressBarMin = ImVec2(NotifyPos.Min.x, NotifyPos.Max.y - 3);
                ImVec2 ProgressBarMax = ImVec2(NotifyPos.Min.x + (NotifySize.x * Progress), NotifyPos.Max.y);

                DrawList->AddRectFilled(ProgressBarMin, ProgressBarMax,
                    ImGui::GetColorU32(ImVec4(255.0f / 255.0f, 0.0f / 255.0f, 51.0f / 255.0f, 0.8f)), 2.0f);
            }

            // TEXTO - T�TULO
            auto TitlePos = ImVec2(NotifyPos.Min.x + Padding, NotifyPos.Min.y + Padding);
            DrawList->AddText(TitlePos,
                ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)),
                Notify.GetTitle().c_str());

            // TEXTO - DESCRI��O
            auto DescPos = ImVec2(NotifyPos.Min.x + Padding, TitlePos.y + TitleTxtSize.y + 2.f);
            DrawList->AddText(DescPos,
                ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f)),
                Notify.GetDescription().c_str());

            ImGui::PopStyleVar();

            // ATUALIZAR PR�XIMA ALTURA
            NextHeight += NotifySize.y + 5.f;
        }
    }

    inline void Send(std::string Description, time_t ExpireTime = 4000)
    {
        NotifyManager::NotifyClass Notify(NotifyManager::eType::Info, ExpireTime);
        Notify.SetTitle("Advanced");
        Notify.SetDescription(Description);
        NotifyList.push_back(Notify);

    }


    inline void ClearAll()
    {
        NotifyList.clear();
    }

}