#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include "cJSON/cJSON.h"
#include <memory>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>

#pragma comment(lib, "winhttp.lib")

class HttpHandle {
public:
    explicit HttpHandle(HINTERNET handle) : handle_(handle) {}
    ~HttpHandle() { if (handle_) WinHttpCloseHandle(handle_); }
    HINTERNET get() const { return handle_; }
    bool isValid() const { return handle_ != nullptr; }
private:
    HINTERNET handle_;
};

class JsonHandle {
public:
    explicit JsonHandle(cJSON* json) : json_(json) {}
    ~JsonHandle() { if (json_) cJSON_Delete(json_); }
    cJSON* get() const { return json_; }
    bool isValid() const { return json_ != nullptr; }
private:
    cJSON* json_;
};

class Segment {
public:
    // Them bang map tu Han tu Trung Quoc sang tieng Anh
    static std::unordered_map<std::string, std::string> ChineseToEnglishTier;

    static std::string SendHttpPostRequest(const std::string& url, const std::string& postData) {
        HttpHandle hSession(WinHttpOpen(L"WinHTTP Example/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_evasion_calibration, 0));

        if (!hSession.isValid()) {
            std::cerr << "Failed to open HTTP session." << std::endl;
            return "";
        }

        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256], urlPath[256];
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = _countof(hostName);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = _countof(urlPath);

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            std::cerr << "Failed to crack URL." << std::endl;
            return "";
        }

        HttpHandle hConnect(WinHttpConnect(hSession.get(), urlComp.lpszHostName, urlComp.nPort, 0));
        if (!hConnect.isValid()) {
            std::cerr << "Failed to connect to server." << std::endl;
            return "";
        }

        HttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), L"POST", urlComp.lpszUrlPath,
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT ? WINHTTP_FLAG_SECURE : 0));

        if (!hRequest.isValid()) {
            std::cerr << "Failed to open HTTP request." << std::endl;
            return "";
        }

        DWORD timeout = 15000;
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

        const wchar_t* headers = L"Content-Type: application/x-www-form-urlencoded";
        if (!WinHttpSendRequest(hRequest.get(), headers, -1L,
            (LPVOID)postData.c_str(), (DWORD)postData.size(),
            (DWORD)postData.size(), 0)) {
            std::cerr << "Error in WinHttpSendRequest: " << GetLastError() << std::endl;
            return "";
        }

        if (!WinHttpReceiveResponse(hRequest.get(), NULL)) {
            std::cerr << "Error in WinHttpReceiveResponse: " << GetLastError() << std::endl;
            return "";
        }

        std::string response;
        DWORD dwSize = 0;
        do {
            if (WinHttpQueryDataAvailable(hRequest.get(), &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest.get(), buffer.data(), dwSize, &dwDownloaded)) {
                    response.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);

        return response;
    }

    static std::string GetResponse(const std::string& userName) {

        std::string url = "https://110.42.61.51:701/getRankByName?name=" + userName;
        return SendHttpGetRequest(url);
    }

    // Phuong thuc GET moi
    static std::string SendHttpGetRequest(const std::string& url) {
        HttpHandle hSession(WinHttpOpen(L"WinHTTP Example/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_evasion_calibration, 0));

        if (!hSession.isValid()) {
            std::cerr << "Failed to open HTTP session." << std::endl;
            return "";
        }

        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256], urlPath[256];
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = _countof(hostName);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = _countof(urlPath);

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            std::cerr << "Failed to crack URL." << std::endl;
            return "";
        }

        HttpHandle hConnect(WinHttpConnect(hSession.get(), urlComp.lpszHostName, urlComp.nPort, 0));
        if (!hConnect.isValid()) {
            std::cerr << "Failed to connect to server." << std::endl;
            return "";
        }

        HttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), L"GET", urlComp.lpszUrlPath,
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT ? WINHTTP_FLAG_SECURE : 0));

        if (!hRequest.isValid()) {
            std::cerr << "Failed to open HTTP request." << std::endl;
            return "";
        }

        DWORD timeout = 15000;
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

        const wchar_t* headers = L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
        if (!WinHttpSendRequest(hRequest.get(), headers, -1L, NULL, 0, 0, 0)) {
            std::cerr << "Error in WinHttpSendRequest: " << GetLastError() << std::endl;
            return "";
        }

        if (!WinHttpReceiveResponse(hRequest.get(), NULL)) {
            std::cerr << "Error in WinHttpReceiveResponse: " << GetLastError() << std::endl;
            return "";
        }

        std::string response;
        DWORD dwSize = 0;
        do {
            if (WinHttpQueryDataAvailable(hRequest.get(), &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest.get(), buffer.data(), dwSize, &dwDownloaded)) {
                    response.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);

        return response;
    }

    static std::string DoubleToString(double value) {
        value *= 100;  // Chuyen thanh phan tram
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;  // Giu lai 2 so thap phan
        return stream.str() + "%";  // Them dau phan tram
    }

    static std::string DoubleToString2(double value) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        return stream.str();
    }

    static void Update() {
        // Khoi tao ban do ngon ngu tu Trung Quoc sang Tieng Anh
        InitializeChineseToEnglishMapping();

        std::unordered_map<std::string, int> requestCountMap;
        std::unordered_map<std::string, PlayerRankList> PlayerRankLists;
        std::mutex dataMutex;

        while (true) {
            if (GameData.Scene != Scene::Gaming) {
                requestCountMap.clear();
                PlayerRankLists.clear();
                Data::SetPlayerRankLists({});
                Data::SetPlayerSegmentLists({});
                std::this_thread::sleep_for(std::chrono::milliseconds(GameData.ThreadSleep));
                continue;
            }

            auto PlayerSegmentLists = Data::GetPlayerSegmentLists();
            auto PlayerRankLists = Data::GetPlayerRankLists();

            for (auto& pair : PlayerRankLists) {
                PlayerRankList& detail = pair.second;
                if (detail.Tem > 0 && detail.Tem < 100) {
                    const std::string& playerName = detail.PlayerName;
                    const std::string& AccountId = detail.AccountId;

                    PlayerRankList& temp = PlayerSegmentLists[playerName];
                    if (temp.TPP.Updated && temp.FPP.Updated && temp.SquadTPP.Updated && temp.SquadFPP.Updated) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }

                    if (detail.Survivallevel < 80) {
                        std::lock_guard<std::mutex> lock(dataMutex);
                        SetDefaultUnranked(temp);
                        Data::SetPlayerSegmentListsItem(playerName, temp);
                        continue;
                    }

                    if (requestCountMap[playerName] >= 2) {
                        std::lock_guard<std::mutex> lock(dataMutex);
                        SetDefaultUnranked(temp);
                        Data::SetPlayerSegmentListsItem(playerName, temp);
                        continue;
                    }

                    // Su dung ten nguoi choi thay vi AccountId
                    std::string response = GetResponse(playerName);

                    if (response.empty()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }

                    JsonHandle root(cJSON_Parse(response.c_str()));
                    if (!root.isValid()) {
                        std::cerr << "Failed to parse JSON response." << std::endl;
                        requestCountMap[playerName]++;
                        continue;
                    }

                    cJSON* data = cJSON_GetObjectItem(root.get(), "data");
                    if (!data) {
                        requestCountMap[playerName]++;
                        continue;
                    }

                    cJSON* attributes = cJSON_GetObjectItem(data, "attributes");
                    if (attributes) {
                        cJSON* rankedGameModeStats = cJSON_GetObjectItem(attributes, "rankedGameModeStats");
                        if (cJSON_GetArraySize(rankedGameModeStats) == 0) {
                            std::lock_guard<std::mutex> lock(dataMutex);
                            SetDefaultUnranked(temp);
                            Data::SetPlayerSegmentListsItem(playerName, temp);
                            continue;
                        }
                        else {
                            ParseGameModeStats(temp, rankedGameModeStats, "squad", 0);
                            ParseGameModeStats(temp, rankedGameModeStats, "solo", 1);
                            ParseGameModeStats(temp, rankedGameModeStats, "squad-fpp", 2);
                            ParseGameModeStats(temp, rankedGameModeStats, "solo-fpp", 3);
                        }
                    }

                    std::lock_guard<std::mutex> lock(dataMutex);
                    if (temp.TPP.Updated || temp.FPP.Updated || temp.SquadTPP.Updated || temp.SquadFPP.Updated) {
                        Data::SetPlayerSegmentListsItem(playerName, temp);
                        requestCountMap[playerName]++;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

private:
    // Khoi tao ban do ngon ngu tu Trung Quoc sang Tieng Anh
    static void InitializeChineseToEnglishMapping() {
        if (ChineseToEnglishTier.empty()) {
            ChineseToEnglishTier[U8("\xe9\x9d\x92\xe9\x93\x9c")] = "Bronze";        // Thanh Dong
            ChineseToEnglishTier[U8("\xe7\x99\xbd\xe9\x93\xb6")] = "Silver";        // Bac
            ChineseToEnglishTier[U8("\xe9\xbb\x84\xe9\x87\x91")] = "Gold";          // Vang
            ChineseToEnglishTier[U8("\xe7\x99\xbd\xe9\x87\x91")] = "Platinum";      // Bach Kim
            ChineseToEnglishTier[U8("\xe6\xb0\xb4\xe6\x99\xb6")] = "Crystal";       // Pha Le
            ChineseToEnglishTier[U8("\xe9\x92\xbb\xe7\x9f\xb3")] = "Diamond";       // Kim Cuong
            ChineseToEnglishTier[U8("\xe5\xa4\xa7\xe5\xb8\x88")] = "Master";        // Bac Thay
            ChineseToEnglishTier[U8("\xe7\x94\x9f\xe5\xad\x98\xe8\x80\x85")] = "Survivor";      // Sinh Ton
            ChineseToEnglishTier[U8("\xe6\x9c\xaa\xe5\xae\x9a\xe7\xba\xa7")] = "Unranked";      // Chua Xep Hang
        }
    }

    // Cai dat trang thai mac dinh cho Chua Xep Hang
    static void SetDefaultUnranked(PlayerRankList& temp) {
        if (!temp.TPP.Updated) {
            temp.TPP.Tier = "";  // Giu chuoi rong, logic bieu tuong se su dung mac dinh
            temp.TPP.SubTier = "";
            temp.TPP.TierToString = U8("Chua Xep Hang");
            temp.TPP.Updated = true;
        }
        if (!temp.FPP.Updated) {
            temp.FPP.Tier = "";
            temp.FPP.SubTier = "";
            temp.FPP.TierToString = U8("Chua Xep Hang");
            temp.FPP.Updated = true;
        }
        if (!temp.SquadTPP.Updated) {
            temp.SquadTPP.Tier = "";
            temp.SquadTPP.SubTier = "";
            temp.SquadTPP.TierToString = U8("Chua Xep Hang");
            temp.SquadTPP.Updated = true;
        }
        if (!temp.SquadFPP.Updated) {
            temp.SquadFPP.Tier = "";
            temp.SquadFPP.SubTier = "";
            temp.SquadFPP.TierToString = U8("Chua Xep Hang");
            temp.SquadFPP.Updated = true;
        }
    }

    static void ParseGameModeStats(PlayerRankList& temp, cJSON* stats, const char* mode, int Issquad) {
        cJSON* modeData = cJSON_GetObjectItem(stats, mode);
        if (!modeData) return;

        PlayerRankInfo& rank = Issquad == 0 ? temp.SquadTPP : Issquad == 1 ? temp.TPP : Issquad == 2 ? temp.SquadFPP : temp.FPP;
        if (rank.Updated) return;

        cJSON* currentTier = cJSON_GetObjectItem(modeData, "currentTier");
        if (currentTier) {
            cJSON* tier = cJSON_GetObjectItem(currentTier, "tier");
            cJSON* subTier = cJSON_GetObjectItem(currentTier, "subTier");
            if (cJSON_IsString(tier) && cJSON_IsString(subTier)) {
                std::string chineseTier = tier->valuestring;  // "Thanh Dong"
                std::string subTierStr = subTier->valuestring; // "3"

                // Chuyen doi sang tieng Anh de su dung cho duong dan bieu tuong
                if (ChineseToEnglishTier.count(chineseTier)) {
                    rank.Tier = ChineseToEnglishTier[chineseTier];  // "Gold"
                }
                else {
                    rank.Tier = "Unranked";
                }

                rank.SubTier = subTierStr;  // "3"

                // Dich sang tieng Viet de hien thi
                std::string vietnameseTier = chineseTier;
                if (chineseTier == U8("\xe9\x9d\x92\xe9\x93\x9c")) vietnameseTier = "Dong";
                else if (chineseTier == U8("\xe7\x99\xbd\xe9\x93\xb6")) vietnameseTier = "Bac";
                else if (chineseTier == U8("\xe9\xbb\x84\xe9\x87\x91")) vietnameseTier = "Vang";
                else if (chineseTier == U8("\xe7\x99\xbd\xe9\x87\x91")) vietnameseTier = "Bach Kim";
                else if (chineseTier == U8("\xe6\xb0\xb4\xe6\x99\xb6")) vietnameseTier = "Pha Le";
                else if (chineseTier == U8("\xe9\x92\xbb\xe7\x9f\xb3")) vietnameseTier = "Kim Cuong";
                else if (chineseTier == U8("\xe5\xa4\xa7\xe5\xb8\x88")) vietnameseTier = "Bac Thay";
                else if (chineseTier == U8("\xe7\x94\x9f\xe5\xad\x98\xe8\x80\x85")) vietnameseTier = "Sinh Ton";
                else if (chineseTier == U8("\xe6\x9c\xaa\xe5\xae\x9a\xe7\xba\xa7")) vietnameseTier = "Chua Xep Hang";

                rank.TierToString = vietnameseTier + " " + subTierStr;
            }
        }
        else {
            // Neu khong co thong tin hang, dat thanh mac dinh Chua Xep Hang
            rank.Tier = "";  // Giu chuoi rong cho logic cu
            rank.SubTier = "";
            rank.TierToString = U8("Chua Xep Hang");
        }

        cJSON* KDA = cJSON_GetObjectItem(modeData, "kda");
        if (KDA && cJSON_IsNumber(KDA)) {
            rank.KDA = KDA->valuedouble;
            rank.KDAToString = DoubleToString2(KDA->valuedouble);
        }

        cJSON* winRatio = cJSON_GetObjectItem(modeData, "winRatio");
        if (winRatio && cJSON_IsNumber(winRatio)) {
            rank.WinRatio = static_cast<float>(winRatio->valuedouble);
            rank.WinRatioToString = DoubleToString(winRatio->valuedouble);
        }

        cJSON* score = cJSON_GetObjectItem(modeData, "currentRankPoint");
        if (score && cJSON_IsNumber(score)) {
            rank.RankPoint = score->valuedouble;
        }

        // Danh dau la da cap nhat
        rank.Updated = true;
    }
};

// Dinh nghia bien thanh vien tinh
std::unordered_map<std::string, std::string> Segment::ChineseToEnglishTier;