# 🔐 Tutorial Completo - Sistema de Autenticação

## 📋 Pré-requisitos

1. **Node.js** (v18+): https://nodejs.org/

---

## 🚀 Instalação

### 1. Instalar dependências do projeto

```cmd
cd auth-system
npm install
```

### 2. Iniciar o servidor

```cmd
npm start
```

O servidor vai rodar em: **http://localhost:3000**
Os dados serão salvos localmente na pasta `db/`.

---

## 🔑 Acessando o Painel

1. Abra o navegador em `http://localhost:3000`
2. Login: `drake`
3. Senha: `drake`

---

## 📦 Usando o Painel

### Criar Produto
1. Clique na aba "📦 Produtos"
2. Clique em "+ Criar Produto"
3. Digite o nome (ex: "MeuApp")
4. Clique em "Criar"

### Adicionar Usuário
1. Clique na aba "👥 Usuários"
2. Clique em "+ Adicionar Usuário"
3. Preencha:
   - **Login**: nome do usuário
   - **Senha**: opcional (se não colocar, só precisa do login)
   - **Produto**: selecione o produto
   - **Role**: Member, Owner, Developer ou Friends
   - **Tempo**: 1 dia, 30 dias, 60 dias, 120 dias, 365 dias ou Lifetime

### Editar/Resetar HWID
- Clique em "Editar" para modificar dados
- Clique em "Reset HWID" para permitir novo dispositivo

---

## 💻 Integração com C++ (ImGui)

### Dependências necessárias
Você vai precisar de uma biblioteca HTTP para C++. Recomendo **cpp-httplib** (header-only):
- https://github.com/yhirose/cpp-httplib

Baixe o `httplib.h` e coloque no seu projeto.

### Código de Autenticação

```cpp
#pragma once
#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <httplib.h>
#include <nlohmann/json.hpp> // https://github.com/nlohmann/json

using json = nlohmann::json;

class AuthSystem {
public:
    std::string username;
    std::string expiry;
    std::string role;
    std::string product;
    std::string hwid;
    bool authenticated = false;
    std::string error;

private:
    // ============================================
    // CONFIGURE AQUI A URL DO SEU SERVIDOR
    // ============================================
    std::string serverUrl = "https://seusite.com";  // Mude para sua URL hospedada
    // Exemplos:
    // "https://meuauth.railway.app"
    // "https://auth.meudominio.com"
    // "http://123.456.789.10:3000" (VPS com IP)
    // ============================================
    
    std::string productName;
    std::string userLogin;

public:
    AuthSystem(const std::string& product) : productName(product) {
        hwid = GetHWID();
    }

    // Função para obter HWID (exemplo simples)
    std::string GetHWID() {
        // Implemente sua lógica de HWID aqui
        // Exemplo usando volume serial do disco:
        char volumeName[MAX_PATH];
        char fileSystemName[MAX_PATH];
        DWORD serialNumber;
        DWORD maxComponentLen;
        DWORD fileSystemFlags;
        
        GetVolumeInformationA("C:\\", volumeName, MAX_PATH, &serialNumber,
            &maxComponentLen, &fileSystemFlags, fileSystemName, MAX_PATH);
        
        return std::to_string(serialNumber);
    }

    bool Login(const std::string& login, const std::string& password = "") {
        userLogin = login;
        
        try {
            httplib::Client cli(serverUrl);
            cli.set_connection_timeout(10);
            
            json body;
            body["login"] = login;
            body["password"] = password;
            body["hwid"] = hwid;
            body["product"] = productName;
            
            auto res = cli.Post("/api/auth", body.dump(), "application/json");
            
            if (res && res->status == 200) {
                json response = json::parse(res->body);
                
                if (response["success"].get<bool>()) {
                    username = response["username"].get<std::string>();
                    expiry = response["expiry"].get<std::string>();
                    role = response["role"].get<std::string>();
                    product = response["product"].get<std::string>();
                    authenticated = true;
                    
                    // Iniciar heartbeat em thread separada
                    StartHeartbeat();
                    return true;
                }
            }
            
            if (res) {
                json response = json::parse(res->body);
                error = response["error"].get<std::string>();
            } else {
                error = "Erro de conexão com servidor";
            }
            
        } catch (const std::exception& e) {
            error = e.what();
        }
        
        return false;
    }

    void StartHeartbeat() {
        std::thread([this]() {
            while (authenticated) {
                SendHeartbeat();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        }).detach();
    }

    void SendHeartbeat() {
        try {
            httplib::Client cli(serverUrl);
            json body;
            body["login"] = userLogin;
            body["product"] = productName;
            body["hwid"] = hwid;
            cli.Post("/api/heartbeat", body.dump(), "application/json");
        } catch (...) {}
    }

    void Logout() {
        if (!authenticated) return;
        
        try {
            httplib::Client cli(serverUrl);
            json body;
            body["login"] = userLogin;
            body["product"] = productName;
            body["hwid"] = hwid;
            cli.Post("/api/logout", body.dump(), "application/json");
        } catch (...) {}
        
        authenticated = false;
    }

    ~AuthSystem() {
        Logout();
    }
};
```


### Exemplo de Uso com ImGui

```cpp
#include "AuthSystem.h"

// Variáveis globais
AuthSystem* auth = nullptr;
char loginInput[64] = "";
char passwordInput[64] = "";
bool showLoginWindow = true;
bool showMainApp = false;

void InitAuth() {
    // Substitua "MeuApp" pelo nome do seu produto
    auth = new AuthSystem("MeuApp");
}

void RenderLoginWindow() {
    if (!showLoginWindow) return;
    
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoCollapse);
    
    ImGui::Text("Login:");
    ImGui::InputText("##login", loginInput, sizeof(loginInput));
    
    ImGui::Text("Senha:");
    ImGui::InputText("##password", passwordInput, sizeof(passwordInput), ImGuiInputTextFlags_Password);
    
    if (ImGui::Button("Entrar", ImVec2(150, 30))) {
        if (auth->Login(loginInput, passwordInput)) {
            showLoginWindow = false;
            showMainApp = true;
        }
    }
    
    if (!auth->error.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Erro: %s", auth->error.c_str());
    }
    
    ImGui::End();
}

void RenderMainApp() {
    if (!showMainApp || !auth->authenticated) return;
    
    ImGui::Begin("Meu App");
    
    // Suas informações de usuário (como você pediu)
    ImGui::Text("Username:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(145 / 255.f, 48 / 255.f, 255 / 255.f, 1.f), "%s", auth->username.c_str());
    
    ImGui::Text("Expiry:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(145 / 255.f, 48 / 255.f, 255 / 255.f, 1.f), "%s", auth->expiry.c_str());
    
    ImGui::Text("Role:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(145 / 255.f, 48 / 255.f, 255 / 255.f, 1.f), "%s", auth->role.c_str());
    
    ImGui::Text("Product:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(145 / 255.f, 48 / 255.f, 255 / 255.f, 1.f), "%s", auth->product.c_str());
    
    ImGui::Separator();
    
    // Resto do seu app aqui...
    
    ImGui::End();
}

// No seu loop principal
void Render() {
    RenderLoginWindow();
    RenderMainApp();
}

// Ao fechar o app
void Cleanup() {
    if (auth) {
        delete auth;
        auth = nullptr;
    }
}
```

---

## 🌐 Colocando Online (Opcional)

Para usar pela internet, você pode hospedar em:

### Opção 1: Railway (Grátis)
1. Crie conta em https://railway.app
2. Conecte seu GitHub
3. Faça upload do projeto
4. Railway vai detectar Node.js automaticamente
5. Adicione MongoDB como serviço

### Opção 2: VPS (DigitalOcean, Vultr, etc)
1. Instale Node.js e MongoDB no servidor
2. Clone o projeto
3. Use PM2 para manter rodando:
```bash
npm install -g pm2
pm2 start server.js
pm2 save
```

### Atualize a URL no C++
No arquivo `AuthSystem.h`, mude a linha:
```cpp
std::string serverUrl = "https://seusite.com";
```
Para a URL do seu servidor hospedado, exemplo:
```cpp
std::string serverUrl = "https://meuauth.railway.app";
```

---

## 📡 Endpoints da API

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| POST | `/api/auth` | Autenticar usuário (app C++) |
| POST | `/api/heartbeat` | Manter usuário online |
| POST | `/api/logout` | Marcar usuário offline |
| POST | `/api/admin/login` | Login do admin |
| GET | `/api/users` | Listar usuários |
| POST | `/api/users` | Criar usuário |
| PUT | `/api/users/:id` | Editar usuário |
| DELETE | `/api/users/:id` | Deletar usuário |
| POST | `/api/users/:id/reset-hwid` | Resetar HWID |
| GET | `/api/products` | Listar produtos |
| POST | `/api/products` | Criar produto |
| DELETE | `/api/products/:id` | Deletar produto |

---

## ⚠️ Dicas de Segurança

1. **Mude o JWT_SECRET** no `server.js`
2. **Use HTTPS** em produção
3. **Mude a senha do admin** após primeiro login
4. **Faça backup do MongoDB** regularmente

---

## 🐛 Problemas Comuns

**MongoDB não conecta:**
- Verifique se o serviço está rodando
- Windows: `services.msc` → MongoDB Server

**HWID não registra:**
- Verifique se a função GetHWID() está retornando valor

**Erro de CORS:**
- O servidor já tem CORS habilitado, mas se hospedar em domínio diferente, ajuste as configurações

---

Pronto! Agora você tem um sistema completo de autenticação com painel admin. 🎉
