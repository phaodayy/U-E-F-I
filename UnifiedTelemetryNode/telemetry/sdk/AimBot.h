#pragma once
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"
#include "common/Entitys.h"
#include "utils/Driver.h"
#include "utils/UsermodeMouse.h"
#include <diagnostic_node/LineTraceHook.h>
#include <diagnostic_node/LineTrace.h>
#include <diagnostic_node/VisibleCheck.h>
#include <Common/Offset.h>
#include "MouseDispatcher.h"
//#include <diagnostic_node/Mortar.h>
#include <set>

#define MAX_flt			(3.402823466e+38F)
#define SMALL_NUMBER		(1.e-8f)

constexpr inline auto NAME_None = FName(0, 0);
constexpr inline auto INDEX_NONE = -1;

// Bien toan cuc va static
WeaponData g_currentWeaponData;
float g_remainMouseX = 0.0f;
float g_remainMouseY = 0.0f;
float g_autoSwitchTargetStartTime = 0;
float g_recoilTimeStartTime = 0;
FVector g_recoilLocation;
float g_lineTraceSingleRecoilTimeStartTime = 0;
FVector g_lineTraceSingleRecoilLocation;
uint64_t g_lastCurrentWeapon = 0;
float g_automaticShootingTime = 0;
float g_lastRandomTime = 0.0f;
float g_randomXSpeedFactor = 1.0f;
float g_randomYSpeedFactor = 1.0f;
float g_randomInitialValueFactor = 1.0f;

// Bien static de luu tru xuong duoc chon ngau nhien
static std::vector<int> g_selectedRandomBones;
static float g_lastRandomBoneSwitch = 0;
static int g_currentRandomBoneIndex = 0;

// Lop dieu khien PID de lam muot chuyen dong chuot
class PIDController {
public:
    double kp, ki, kd;
    double last_error = 0;
    double integral = 0;

    PIDController(double p = 0.5, double i = 0.0, double d = 0.1) : kp(p), ki(i), kd(d) {}

    void init(double p, double i, double d) {
        kp = p; ki = i; kd = d;
        last_error = 0;
        integral = 0;
    }

    double compute(double error, double delta_time = 0.01) {
        integral += error * delta_time;
        double derivative = (error - last_error) / delta_time;
        double output = kp * error + ki * integral + kd * derivative;
        last_error = error;
        return output;
    }

    void clear() {
        last_error = 0;
        integral = 0;
    }
};

static PIDController PidX(0.4, 0.01, 0.05);
static PIDController PidY(0.4, 0.01, 0.05);
static bool g_isPIDInitialized = false;

// Ham kiem tra vi tri xuong va tinh kha dung
bool GetLocation1(const Player& targetCharacter, int firstBoneIndex, EBoneIndex* visibilityBoneIndex)
{
    if (!targetCharacter.Skeleton) return false;
    // Danh sach xuong uu tien neu diem ban dau khong hien thi
    static const std::vector<int> priorityBones = { EBoneIndex::ForeHead, EBoneIndex::Head, EBoneIndex::Neck_01, EBoneIndex::Spine_03, EBoneIndex::Pelvis, EBoneIndex::Spine_01 }; // Head, Neck, Chest, Pelvis...

    if (targetCharacter.Skeleton->VisibleBones[firstBoneIndex])
    {
        *visibilityBoneIndex = (EBoneIndex)firstBoneIndex;
        return true;
    }
    
    for (int bone : priorityBones)
    {
        if (targetCharacter.Skeleton->VisibleBones[bone])
        {
            *visibilityBoneIndex = (EBoneIndex)bone;
            return true;
        }
    }
    return false;
}

// Tinh thoi gian bay don gian
double CalculateFlightTimeSimple(double targetDistance, double initialSpeed = 15.0)
{
    if (targetDistance <= 0 || initialSpeed <= 0)
    {
        return 0.0; // Dau vao khong hop le
    }
    return (targetDistance / initialSpeed) * 1000.0; // Thoi gian = Khoang cach / Toc do -> mili giay
}

// Tinh do cao toi da
double CalculateMaxHeight(double targetDistance, double initialSpeed = 15.0, double gravity = 9.81)
{
    if (targetDistance <= 0 || initialSpeed <= 0) return 0.0;
    if (targetDistance < 50.0 || targetDistance > 80.0) return 0.0;

    double maxDistance = (initialSpeed * initialSpeed) / gravity;
    if (targetDistance > maxDistance) {
        double minSpeed = sqrt(targetDistance * gravity);
        return (initialSpeed * initialSpeed - minSpeed * minSpeed) / (2 * gravity);
    }

    double sin2theta = (targetDistance * gravity) / (initialSpeed * initialSpeed);
    double theta = M_PI_2 - 0.5 * asin(sin2theta);
    return (initialSpeed * initialSpeed) * pow(sin(theta), 2) / (2 * gravity);
}

std::atomic<bool> g_isGrenade = false;
std::atomic<bool> g_isGrenadeEx = false;
float g_highTime = 0.f;

// Tinh toan luu dan
void ClacGrenade()
{
    g_isGrenade = true;
    Sleep(g_highTime);
    KmBoxNet::PopUpTheLeft();
    Sleep(2000); // Do tre chuyen doi precision_calibration luu dan
    g_isGrenade = false;
    g_isGrenadeEx = false;
}

// Kich hoat chuot trai
void TriggerMouseLeft(int durationMs = 30)
{
    if (!GameData.Config.precision_calibration.Connected) return;

    switch (GameData.Config.precision_calibration.Controller) {
    case 4:
        Driver::Click();
        break;
    case 5:
        UsermodeMouse::Click();
        break;
    }
}


// Lay ngau nhien bo phan co the
int GetRandomBodyPart(precision_calibrationConfig& config) {
    if (!config.RandomBodyParts) return -1;  // Neu chua bat random bo phan co the, tra ve -1

    float currentTime = GetTickCount64();

    // Lan dau chay hoac khi cau hinh thay doi, tinh lai danh sach xuong ngau nhien
    if (g_selectedRandomBones.empty() || (currentTime - g_lastRandomBoneSwitch > 10000)) {
        g_selectedRandomBones.clear();

        // Thong ke so luong bo phan da bat
        std::vector<int> enabledParts;
        for (int i = 0; i < 17; i++) {
            if (config.RandomBodyPartsList[i]) {
                enabledParts.push_back(BoneIndex[i]);
            }
        }

        // Neu khong co bo phan nao duoc bat, tra ve -1
        if (enabledParts.empty()) return -1;

        // Chon toi da config.RandomBodyPartCount xuong, neu khong du thi chon tat ca
        int selectCount = (enabledParts.size() < config.RandomBodyPartCount) ?
            (int)enabledParts.size() : config.RandomBodyPartCount;

        // Chon ngau nhien selectCount xuong
        while (g_selectedRandomBones.size() < (size_t)selectCount) {
            if (enabledParts.empty()) break;

            int randomIndex = rand() % enabledParts.size();
            g_selectedRandomBones.push_back(enabledParts[randomIndex]);
            enabledParts.erase(enabledParts.begin() + randomIndex);
        }

        g_currentRandomBoneIndex = 0;
    }

    // Neu danh sach xuong rong, tra ve -1
    if (g_selectedRandomBones.empty()) return -1;

    // Chuyen muc tieu xuong dua tren toc do ngau nhien
    if (currentTime - g_lastRandomBoneSwitch > config.RandomSpeed) {
        g_currentRandomBoneIndex = rand() % g_selectedRandomBones.size();
        g_lastRandomBoneSwitch = currentTime;
    }

    return g_selectedRandomBones[g_currentRandomBoneIndex];
}

extern int targetScale2;
extern bool adjusting;

class precision_calibration
{
public:
    static void setTargetScale(int scaleFactor) {
        targetScale2 = scaleFactor;
        adjusting = true;
    }

    // Thuc hien cuon chuot va log
    static void executeScroll(int steps) {
        steps = std::clamp(steps, 0, 100);  // Gioi han trong pham vi hop ly

        for (int i = 0; i < steps; i++) {
            mouse_scroll_down();
        }
    }

    // Thuc hien dieu chinh
    static void adjust(int targetScale) {
        if (!adjusting) return;

        for (size_t i = 0; i < 85; i++) {
            mouse_scroll_up();
        }
        static std::vector<std::pair<int, int>> s_vMap = {
             {121 ,81}, {122 ,81}, {123 ,81}, {124 ,81}, {125 ,81},
             {128 ,80}, {129 ,80}, {130 ,80}, {131 ,80}, {132 ,80}, {133 ,80}, {134 ,80}, {135 ,80}, {136 ,80}, {137 ,80}, {138 ,80},
             {700 ,1},
        };
        // Luu y: Do dai cua vector map, giu nguyen data de dam bao logic

        if (s_vMap.empty()) return;

        if (targetScale < s_vMap.front().first) return;
        if (targetScale >= s_vMap.back().first) {
            executeScroll(s_vMap.back().second);
            return;
        }

        size_t low = 0;
        size_t high = s_vMap.size() - 1;
        size_t index = 0;

        while (low <= high) {
            index = low + (high - low) / 2;
            const auto& current = s_vMap[index];

            if (std::abs(current.first - targetScale) <= 8) {
                executeScroll(current.second);
                return;
            }

            if (current.first == targetScale) {
                executeScroll(current.second);
                return;
            }

            if (targetScale < current.first) {
                if (index > 0 && targetScale > s_vMap[index - 1].first) {
                    break;
                }
                high = index - 1;
            }
            else {
                low = index + 1;
            }
        }

        const auto& lower = s_vMap[index - 1];
        const auto& upper = s_vMap[index];
        double ratio = static_cast<double>(targetScale - lower.first) / (upper.first - lower.first);
        int cnt = static_cast<int>(lower.second + ratio * (upper.second - lower.second) + 0.5);
        executeScroll(cnt);
    }
   
    static void StopAiming(bool useSleep = true)
    {
        g_isMortars = false;
        GameData.precision_calibration.Lock = false;
        GameData.precision_calibration.Target = 0;
        g_remainMouseX = 0.f;
        g_remainMouseY = 0.f;
        g_autoSwitchTargetStartTime = 0;
        g_recoilTimeStartTime = 0;
        g_recoilLocation = { 0.f, 0.f, 0.f };
        g_lineTraceSingleRecoilTimeStartTime = 0;
        g_lineTraceSingleRecoilLocation = { 0.f, 0.f, 0.f };
        g_lastCurrentWeapon = 0;
        Data::SetEnemyInfoMap({});
    }

    static void CycleTime(float minTime, float maxTime, float& inTime, int& cycleCount)
    {
        float initTime = inTime;
        float duration = maxTime - minTime;

        if (inTime > maxTime)
        {
            cycleCount = FloorToInt((maxTime - inTime) / duration);
            inTime = inTime + duration * cycleCount;
        }
        else if (inTime < minTime)
        {
            cycleCount = FloorToInt((inTime - minTime) / duration);
            inTime = inTime - duration * cycleCount;
        }

        if (inTime == maxTime && initTime < minTime)
        {
            inTime = minTime;
        }

        if (inTime == minTime && initTime > maxTime)
        {
            inTime = maxTime;
        }

        cycleCount = Abs(cycleCount);
    }

    static void RemapTimeValue(float& inTime, float& cycleValueOffset, FRichCurve richCurve, int keysNum, std::vector<FRichCurveKey> keys)
    {
        const int32 numKeys = keysNum;

        if (numKeys < 2)
        {
            return;
        }

        if (inTime <= keys[0].Time)
        {
            if (richCurve.PreInfinityExtrap != RCCE_Linear && richCurve.PreInfinityExtrap != RCCE_Constant)
            {
                float minTime = keys[0].Time;
                float maxTime = keys[numKeys - 1].Time;

                int cycleCount = 0;
                CycleTime(minTime, maxTime, inTime, cycleCount);

                if (richCurve.PreInfinityExtrap == RCCE_CycleWithOffset)
                {
                    float dV = keys[0].Value - keys[numKeys - 1].Value;
                    cycleValueOffset = dV * cycleCount;
                }
                else if (richCurve.PreInfinityExtrap == RCCE_Oscillate)
                {
                    if (cycleCount % 2 == 1)
                    {
                        inTime = minTime + (maxTime - inTime);
                    }
                }
            }
        }
        else if (inTime >= keys[numKeys - 1].Time)
        {
            if (richCurve.PostInfinityExtrap != RCCE_Linear && richCurve.PostInfinityExtrap != RCCE_Constant)
            {
                float minTime = keys[0].Time;
                float maxTime = keys[numKeys - 1].Time;

                int cycleCount = 0;
                CycleTime(minTime, maxTime, inTime, cycleCount);

                if (richCurve.PostInfinityExtrap == RCCE_CycleWithOffset)
                {
                    float dV = keys[numKeys - 1].Value - keys[0].Value;
                    cycleValueOffset = dV * cycleCount;
                }
                else if (richCurve.PostInfinityExtrap == RCCE_Oscillate)
                {
                    if (cycleCount % 2 == 1)
                    {
                        inTime = minTime + (maxTime - inTime);
                    }
                }
            }
        }
    }

    static void precision_calibrationAPI_SG(FVector2D moveXY, precision_calibrationConfig config)
    {
        if (moveXY.X == 0 && moveXY.Y == 0) {
            return;
        }

        float mouseX = moveXY.X * config.XSpeed / 100.0f;
        float mouseY = moveXY.Y * config.YSpeed / 100.0f;

        if (abs(mouseX) > 0 || abs(mouseY) > 0) {
            Move(mouseX, mouseY);
        }

        if (abs(moveXY.X) < config.Threshold && abs(moveXY.Y) < config.Threshold) {
            TriggerMouseLeft();
        }
    }

    static float Eval(float inTime, float inDefaultValue, FRichCurve richCurve, int keysNum, std::vector<FRichCurveKey> keys)
    {
        float cycleValueOffset = 0;
        RemapTimeValue(inTime, cycleValueOffset, richCurve, keysNum, keys);

        const int32 numKeys = keysNum;
        float interpVal = richCurve.DefaultValue == MAX_flt ? inDefaultValue : richCurve.DefaultValue;

        if (numKeys == 0)
        {
        }
        else if (numKeys < 2 || (inTime <= keys[0].Time))
        {
            if (richCurve.PreInfinityExtrap == RCCE_Linear && numKeys > 1)
            {
                float dT = keys[1].Time - keys[0].Time;

                if (IsNearlyZero(dT))
                {
                    interpVal = keys[0].Value;
                }
                else
                {
                    float dV = keys[1].Value - keys[0].Value;
                    float slope = dV / dT;

                    interpVal = slope * (inTime - keys[0].Time) + keys[0].Value;
                }
            }
            else
            {
                interpVal = keys[0].Value;
            }
        }
        else if (inTime < keys[numKeys - 1].Time)
        {
            int32 first = 1;
            int32 last = numKeys - 1;
            int32 count = last - first;

            while (count > 0)
            {
                int32 step = count / 2;
                int32 middle = first + step;

                if (inTime >= keys[middle].Time)
                {
                    first = middle + 1;
                    count -= step + 1;
                }
                else
                {
                    count = step;
                }
            }

            int32 interpNode = first;
            const float diff = keys[interpNode].Time - keys[interpNode - 1].Time;

            if (diff > 0.f && keys[interpNode - 1].InterpMode != RCIM_Constant)
            {
                const float alpha = (inTime - keys[interpNode - 1].Time) / diff;
                const float p0 = keys[interpNode - 1].Value;
                const float p3 = keys[interpNode].Value;

                if (keys[interpNode - 1].InterpMode == RCIM_Linear)
                {
                    interpVal = Lerp(p0, p3, alpha);
                }
                else
                {
                    const float oneThird = 1.0f / 3.0f;
                    const float p1 = p0 + (keys[interpNode - 1].LeaveTangent * diff * oneThird);
                    const float p2 = p3 - (keys[interpNode].ArriveTangent * diff * oneThird);

                    interpVal = BezierInterp(p0, p1, p2, p3, alpha);
                }
            }
            else
            {
                interpVal = keys[interpNode - 1].Value;
            }
        }
        else
        {
            if (richCurve.PostInfinityExtrap == RCCE_Linear)
            {
                float dT = keys[numKeys - 2].Time - keys[numKeys - 1].Time;

                if (IsNearlyZero(dT))
                {
                    interpVal = keys[numKeys - 1].Value;
                }
                else
                {
                    float dV = keys[numKeys - 2].Value - keys[numKeys - 1].Value;
                    float slope = dV / dT;

                    interpVal = slope * (inTime - keys[numKeys - 1].Time) + keys[numKeys - 1].Value;
                }
            }
            else
            {
                interpVal = keys[numKeys - 1].Value;
            }
        }
        return interpVal + cycleValueOffset;
    }

    static void SimulateWeaponTrajectory(FVector direction, float distance, float trajectoryGravityZ,
        float ballisticDragScale, float ballisticDropScale,
        float bds, float simulationSubstepTime, float vDragCoefficient,
        FRichCurve richCurve, int keysNum, std::vector<FRichCurveKey> keys,
        float& bulletDrop, float& travelTime)
    {
        float travelDistance = 0.0f;
        float currentDrop = 0.0f;
        bulletDrop = 0.0f;
        travelTime = 0.0f;

        direction.Normalize();
        direction = direction * 100.0f;

        while (1)
        {
            float bulletSpeed = Eval(travelDistance * bds * ballisticDragScale, 0.0, richCurve, keysNum, keys);

            FVector velocity = direction * bulletSpeed;
            velocity.Z += currentDrop;

            FVector acceleration = velocity * simulationSubstepTime;
            float accelerationLen = acceleration.Length() / 100.0f;
            if (travelDistance + accelerationLen > distance)
            {
                float remainDistance = distance - travelDistance;
                float accelerationSpeed = accelerationLen / simulationSubstepTime;
                float remainTime = remainDistance / accelerationSpeed;

                travelTime += remainTime;
                bulletDrop += remainTime * currentDrop;
                break;
            }
            travelDistance += accelerationLen;
            travelTime += simulationSubstepTime;
            bulletDrop += simulationSubstepTime * currentDrop;
            currentDrop += simulationSubstepTime * trajectoryGravityZ * 100 * vDragCoefficient * ballisticDropScale;
        }
    }

#include <set>

// ... (giữ nguyên các phần trước đó cho đến GetDragForce)

    static float GetDragForce(float distance) {
        std::string weaponName = GameData.LocalPlayerInfo.WeaponEntityInfo.Name;
        
        // Nhóm 1: Các loại súng có đạn đạo tiêu chuẩn (AR, SR, SMG phổ biến)
        static std::set<std::string> GunGroupNodes = {
            "WeapAK47_C", "WeapLunchmeats_AK47_C", "WeapGroza_C", "WeapBerylM762_C", "WeapMini14_C", "WeapQBU88_C", 
            "Weapon_G36C_C", "WeapKar98k_C", "WeapMosinNagant_C", "WeapJulies_Kar98k_C", "WeapM24_C", "WeapAWM_C",
            "WeapHK416_C", "WeapDuncans_M416_C", "WeapK2_C", "WeapSCAR-L_C", "WeapM16A4_C", "WeapQBZ95_C", "WeapAUG_C",
            "Weapon_M249_C", "WeaponMk14_C", "Weapon_L6_C", "WeapMG3_C", "WeapACE32_C", "WeapMads_QBU88_C", "Weapon_Mosin_C"
        };

        // Nhóm 2: Các loại súng có độ rơi cao hoặc DMR đặc thù (SKS, SLR, Mk12, Dragunov)
        static std::set<std::string> GunGroupHighDrop = {
            "WeapSKS_C", "WeapFNFal_C", "Weapon_Mk47Mutant_C", "WeapMk12_C", "WeapDragunov_C", "WeapVSS_C", "WeapWin94_C"
        };

        float dragFactor = 1.0f;

        if (GunGroupNodes.count(weaponName)) {
            if (distance <= 50) dragFactor = 0.41f;
            else if (distance <= 100) dragFactor = 0.42f;
            else if (distance <= 150) dragFactor = 0.43f;
            else if (distance <= 200) dragFactor = 0.44f;
            else if (distance <= 300) dragFactor = 0.60f;
            else if (distance <= 400) dragFactor = 0.95f;
            else if (distance <= 600) dragFactor = 1.45f;
            else if (distance <= 800) dragFactor = 1.63f;
            else dragFactor = 2.10f;
        }
        else if (GunGroupHighDrop.count(weaponName)) {
            if (distance <= 100) dragFactor = 0.42f;
            else if (distance <= 200) dragFactor = 0.45f;
            else if (distance <= 300) dragFactor = 0.75f;
            else if (distance <= 400) dragFactor = 1.10f;
            else if (distance <= 500) dragFactor = 1.95f;
            else if (distance <= 600) dragFactor = 2.45f;
            else dragFactor = 3.50f;
        }
        else {
            // Mặc định cho súng lục hoặc vũ khí không xác định
            dragFactor = (distance <= 100) ? 1.11f : 1.50f;
        }

        return dragFactor;
    }

    static FVector GetAdvancedPrediction(const FVector& gunLocation, const FVector& targetPos, const FVector& targetVelocity, float initialSpeed, float gravityZ) {
        if (initialSpeed <= 0) initialSpeed = 800.0f;
        
        float distance = gunLocation.Distance(targetPos) / 100.0f;
        float travelTime = distance / initialSpeed;
        
        // 1. Dự đoán vị trí tương lai dựa trên vận tốc kẻ địch
        FVector predictedPos = targetPos + (targetVelocity * travelTime);
        
        // 2. Tinh toán Bullet Drop (Trọng lực 9.8m/s^2)
        // Hệ số 0.5 * g * t^2
        float gravityEffect = 0.5f * 9.81f * travelTime * travelTime * 100.0f;
        
        // 3. Áp dụng Drag Force tinh vi theo nhóm súng
        float drag = GetDragForce(distance);
        predictedPos.Z += (gravityEffect * drag);
        
        // 4. Bù trừ Height Over Bore (Nếu có dữ liệu Socket)
        // Thông thường ống ngắm cao hơn nòng 3.5cm - 5cm tùy súng
        float heightOverBore = 3.8f; 
        if (distance < 50.0f) {
            predictedPos.Z += heightOverBore * (1.0f - (distance / 50.0f));
        }

        return predictedPos;
    }

    static float GetPredicted(const FVector& gunLocation, FVector targetPos, FVector targetVelocity, FWeaponTrajectoryConfig trajectoryConfig) {
        const float distance = gunLocation.Distance(targetPos) / 100.0f;
        float travelTime = distance / trajectoryConfig.InitialSpeed;
        
        float drop = 0.5f * 9.81f * travelTime * travelTime * 100.0f;
        drop *= GetDragForce(distance);

        // Không bù rơi ở cự ly quá gần để tránh aim văng
        if (distance <= 10.0f) drop = 0;

        return drop;
    }


    static void PressTheLeft()
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }
        switch (GameData.Config.precision_calibration.Controller) {
        case 4:
            telemetryMemory::MoveMouse(0, 0, 0x0001);
            break;
        case 5:
            UsermodeMouse::PressTheLeft();
            break;
        default:
            return;
        }
    }

    static void PopUpTheLeft()
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }
        switch (GameData.Config.precision_calibration.Controller) {
        case 4:
            telemetryMemory::MoveMouse(0, 0, 0x0002);
            break;
        case 5:
            UsermodeMouse::PopUpTheLeft();
            break;
        default:
            return;
        }
    }


    static void mouse_scroll_up()
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }
        switch (GameData.Config.precision_calibration.Controller) {
        case 5:
            // Scroll usually doesn't need click, but mimic Case 1 if needed
            // UsermodeMouse doesn't have scroll yet, leaving case 5 placeholder
            break;
        default:
            return;
        }
    }

    static void mouse_scroll_down()
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }
        switch (GameData.Config.precision_calibration.Controller) {
        case 5:
            break;
        default:
            return;
        }
    }
    static void simulateClick()
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }
        switch (GameData.Config.precision_calibration.Controller) {
        case 4:
            Driver::Click();
            break;
        case 5:
            UsermodeMouse::Click();
            break;
        default:
            return;
        }
    }

    static void Move(int x, int y)
    {
        if (!GameData.Config.precision_calibration.Connected)
        {
            return;
        }

        switch (GameData.Config.precision_calibration.Controller) {
        case 4:
            Driver::Move(x, y);
            break;
        case 5:
            UsermodeMouse::Move(x, y);
            break;
        default:
            return;
        }
    }

    static void precision_calibrationAPI(FVector2D moveXY, precision_calibrationConfig config)
    {
        FVector fMouseXY = { (float)moveXY.X, (float)moveXY.Y, 0.0f };
        fMouseXY.Normalize();

        if (moveXY.X == 0 && moveXY.Y == 0) {
            g_remainMouseX = g_remainMouseY = 0.0f;
            return;
        }

        float initialValue = config.InitialValue;
        // Logic tu dong ngam ngau nhien
        if (config.RandomAim) {
            float currentTime = GetTickCount() / 1000.0f;
            if (currentTime - g_lastRandomTime > (config.RandomInterval / 1000.0f)) {
                // Tao he so ngau nhien moi
                g_randomXSpeedFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * config.RandomFactor;
                g_randomYSpeedFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * config.RandomFactor;
                g_randomInitialValueFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * config.RandomFactor;
                g_lastRandomTime = currentTime;
            }

            // Ap dung he so ngau nhien
            config.XSpeed *= g_randomXSpeedFactor;
            config.YSpeed *= g_randomYSpeedFactor;
            initialValue *= g_randomInitialValueFactor;
        }
        if (GameData.precision_calibration.Type == EntityType::Wheel)
        {
            config.XSpeed = config.AimWheelSpeed;
            config.YSpeed = config.AimWheelSpeed;
        }

        float mouseX = g_remainMouseX + std::clamp((initialValue * (config.XSpeed / 100.0f)) * fMouseXY.X, -(float)abs(moveXY.X), (float)abs(moveXY.X));
        float mouseY = g_remainMouseY + std::clamp((initialValue * (config.YSpeed / 100.0f)) * fMouseXY.Y, -(float)abs(moveXY.Y), (float)abs(moveXY.Y));

        g_remainMouseX = mouseX - truncf(mouseX);
        g_remainMouseY = mouseY - truncf(mouseY);

        if (abs(mouseX) > 0 || abs(mouseY) > 0) {
            Move(mouseX, mouseY);
        }
    }

    static bool GetBoneIsAllFalse(const bool bones[17])
    {
        for (size_t i = 0; i < 17; i++)
        {
            if (bones[i]) {
                return true;
            }
        }
        return false;
    }

    static void GetScopingAttachPointRelativeZ(
        const VMMDLL_SCATTER_HANDLE& hScatter,
        const FTransform weaponComponentToWorld,
        float& scopingAttachPointRelativeZ,
        FTransform& socketWorldTransform,
        FTransform& scopeMeshComponentToWorld
    )
    {
        FRotator scopeSocketRelativeRotation;
        FVector scopeSocketRelativeLocation;
        FVector scopeSocketRelativeScale;

        if (g_currentWeaponData.ScopeSocket)
        {
            mem.AddScatterReadRequest(hScatter, g_currentWeaponData.ScopeAimCameraSocket + Offset::StaticRelativeRotation, (FRotator*)&scopeSocketRelativeRotation);
            mem.AddScatterReadRequest(hScatter, g_currentWeaponData.ScopeAimCameraSocket + Offset::StaticRelativeLocation, (FVector*)&scopeSocketRelativeLocation);
            mem.AddScatterReadRequest(hScatter, g_currentWeaponData.ScopeAimCameraSocket + Offset::StaticRelativeScale, (FVector*)&scopeSocketRelativeScale);
            mem.AddScatterReadRequest(hScatter, g_currentWeaponData.ScopeStaticMeshComponent + Offset::ComponentToWorld, (FTransform*)&scopeMeshComponentToWorld);
            mem.ExecuteReadScatter(hScatter);
        }

        if (g_currentWeaponData.ScopeSocket)
        {
            socketWorldTransform = FTransform(scopeSocketRelativeRotation, scopeSocketRelativeLocation, scopeSocketRelativeScale) * weaponComponentToWorld;
            const float relativeZ_1 = socketWorldTransform.GetRelativeTransform(scopeMeshComponentToWorld).Translation.Z;
            const float relativeZ_2 = scopeMeshComponentToWorld.GetRelativeTransform(weaponComponentToWorld).Translation.Z;
            scopingAttachPointRelativeZ = relativeZ_1 + relativeZ_2;
        }
        else {
            scopingAttachPointRelativeZ = weaponComponentToWorld.GetRelativeTransform(socketWorldTransform).Translation.Z;
        }
    }

    static uint64_t GetStaticMeshComponentScopeType(uint64_t mesh) {
        uint64_t result = 0;
        auto attachedStaticComponentMap = mem.Read<TMap<TEnumAsByte<EWeaponAttachmentSlotID>, uint64_t>>(mesh + Offset::AttachedStaticComponentMap);
        attachedStaticComponentMap.GetValue(EWeaponAttachmentSlotID::UpperRail, result);
        return result;
    }

    static bool FindSocket(uint64_t staticMesh, FName inSocketName, ULONG64& outSocket) {
        if (inSocketName == NAME_None)
            return false;
        auto sockets = mem.Read<TArray<uint64_t>>(staticMesh + Offset::StaticSockets);
        if (!sockets.size()) return false;
        for (const auto& socketPtr : sockets.GetVector()) {

            auto socketName = mem.Read<FName>(socketPtr + Offset::StaticSocketName);
            if (socketName == inSocketName) {
                outSocket = socketPtr;
                return true;
            }
        }
        return false;
    }

    static bool GetSocketByName(uint64_t mesh, FName inSocketName, ULONG64& outSocket)
    {
        uint64_t staticMMesh = mem.Read<uint64_t>(mesh + Offset::StaticMesh);
        if (Utils::ValidPtr(staticMMesh))
            return false;

        return FindSocket(staticMMesh, inSocketName, outSocket);
    }

    static std::pair<float, float> GetBulletDropAndTravelTime(const FVector& gunLocation, const FRotator& gunRotation, const FVector& targetPos,
        float zeroingDistance, float bulletDropAdd, float initialSpeed, float trajectoryGravityZ, float ballisticDragScale,
        float ballisticDropScale, float bds, float simulationSubstepTime, float vDragCoefficient, FRichCurve richCurve, int keysNum, std::vector<FRichCurveKey> keys)
    {
        const float zDistanceToTarget = targetPos.Z - gunLocation.Z;
        const float distanceToTarget = gunLocation.Distance(targetPos) / 100.0f;
        float travelTime = distanceToTarget / initialSpeed;
        float bulletDrop = 0.5f * trajectoryGravityZ * travelTime * travelTime * 100.0f;

        float travelTimeZero = zeroingDistance / initialSpeed;
        float bulletDropZero = 0.5f * trajectoryGravityZ * travelTimeZero * travelTimeZero * 88.0f;

        if (keysNum > 0)
        {
            SimulateWeaponTrajectory(gunRotation.GetUnitVector(), distanceToTarget, trajectoryGravityZ,
                ballisticDragScale, ballisticDropScale,
                bds, simulationSubstepTime,
                vDragCoefficient,
                richCurve, keysNum, keys, bulletDrop, travelTime);


            SimulateWeaponTrajectory(FVector(1.0f, 0.0f, 0.0f), zeroingDistance, trajectoryGravityZ,
                ballisticDragScale, ballisticDropScale, bds, simulationSubstepTime, vDragCoefficient,
                richCurve, keysNum, keys, travelTimeZero, bulletDropZero);
        }

        bulletDrop = fabsf(bulletDrop) - fabsf(bulletDropAdd);
        if (bulletDrop < 0.0f)
            bulletDrop = 0.0f;
        bulletDropZero = fabsf(bulletDropZero) + fabsf(bulletDropAdd);

        const float targetPitch = asinf((zDistanceToTarget + bulletDrop) / 100.0f / distanceToTarget);
        const float zeroPitch = IsNearlyZero(zeroingDistance) ? 0.0f : atan2f(bulletDropZero / 100.0f, zeroingDistance);
        const float finalPitch = targetPitch - zeroPitch;
        const float additiveZ = distanceToTarget * sinf(finalPitch) * 100.0f - zDistanceToTarget;

        return std::pair(additiveZ, travelTime);
    }

    static void GrenadeHwind(float targetDistance, float& projectHinght, float& project)
    {
        if (targetDistance > 10.0f && targetDistance <= 20.0f)
        {
            float t = (targetDistance - 10.0f) / (20.0f - 10.0f);
            projectHinght = -50 + t * (0 - (-50));
            project = 4.2f + t * (3.7f - 4.2f);
        }
        else if (targetDistance > 20.0f && targetDistance <= 25.0f)
        {
            float t = (targetDistance - 20.0f) / (25.0f - 20.0f);
            projectHinght = 0 + t * (0 - 0);
            project = 3.7f + t * (3.4f - 3.7f);
        }
        else if (targetDistance > 25.0f && targetDistance <= 30.0f)
        {
            float t = (targetDistance - 25.0f) / (30.0f - 25.0f);
            projectHinght = 0 + t * (50 - 0);
            project = 3.4f + t * (3.1f - 3.4f);
        }
        else if (targetDistance > 30.0f && targetDistance <= 35.0f)
        {
            float t = (targetDistance - 30.0f) / (35.0f - 30.0f);
            projectHinght = 50 + t * (80 - 50);
            project = 3.1f + t * (2.8f - 3.1f);
        }
        else if (targetDistance > 35.0f && targetDistance <= 40.0f)
        {
            float t = (targetDistance - 35.0f) / (40.0f - 35.0f);
            projectHinght = 80 + t * (500 - 80);
            project = 2.8f + t * (2.5f - 2.8f);
        }
        else if (targetDistance > 40.0f && targetDistance <= 45.0f)
        {
            float t = (targetDistance - 40.0f) / (45.0f - 40.0f);
            projectHinght = 500 + t * (800 - 500);
            project = 2.5f + t * (2.3f - 2.5f);
        }
        else if (targetDistance > 45.0f && targetDistance <= 50.0f)
        {
            float t = (targetDistance - 45.0f) / (50.0f - 45.0f);
            projectHinght = 800 + t * (1400 - 800);
            project = 2.3f + t * (1.9f - 2.3f);
        }
        else if (targetDistance > 50.0f && targetDistance <= 55.0f)
        {
            float t = (targetDistance - 50.0f) / (55.0f - 50.0f);
            projectHinght = 1400 + t * (1800 - 1400);
            project = 1.9f + t * (1.6f - 1.9f);
        }
        else if (targetDistance > 55.0f && targetDistance <= 60.0f)
        {
            float t = (targetDistance - 55.0f) / (60.0f - 55.0f);
            projectHinght = 1800 + t * (2300 - 1800);
            project = 1.6f + t * (1.2f - 1.6f);
        }
        else if (targetDistance > 60.0f && targetDistance <= 65.0f)
        {
            projectHinght = 2300.0f;
            project = 1.2f;
        }
        else
        {
            projectHinght = -50.0f;
            project = 4.2f;
        }
    }

    static void RocketHwind(float targetDistance, float& projectHeight, float& projectTime)
    {
        if (targetDistance <= 35.0f) {
            projectHeight = -100.0f;
            projectTime = 0.4f;
        }
        else if (targetDistance <= 70.0f) {
            projectHeight = -110.0f;
            projectTime = 0.7f;
        }
        else if (targetDistance <= 90.0f) {
            projectHeight = 180.0f;
            projectTime = 1.0f;
        }
        else if (targetDistance <= 100.0f) {
            projectHeight = 360.0f;
            projectTime = 1.3f;
        }
        else if (targetDistance <= 110.0f) {
            projectHeight = 420.0f;
            projectTime = 1.4f;
        }
        else if (targetDistance <= 120.0f) {
            projectHeight = 800.0f;
            projectTime = 1.8f;
        }
        else {
            projectHeight = 1600.0f;
            projectTime = 2.4f;
        }
    }

    static double calculateDistance(double x1, double y1, double x2, double y2) {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }

    // Helper: Cap nhat thong tin vu khi
    static bool UpdateWeaponInfo(VMMDLL_SCATTER_HANDLE hScatter) {
        if (Utils::ValidPtr(g_lastCurrentWeapon) || g_lastCurrentWeapon != GameData.LocalPlayerInfo.CurrentWeapon)
        {
            uint64_t weaponTrajectoryData;
            FWeaponTrajectoryConfig trajectoryConfig;

            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + Offset::WeaponTrajectoryData, (uint64_t*)&weaponTrajectoryData);
            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + Offset::TrajectoryGravityZ, (float*)&g_currentWeaponData.TrajectoryGravityZ);
            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + Offset::FiringAttachPoint, (FName*)&g_currentWeaponData.FiringAttachPoint);
            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + Offset::ScopingAttachPoint, (FName*)&g_currentWeaponData.ScopingAttachPoint);
            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + Offset::Mesh3P, (uint64_t*)&g_currentWeaponData.Mesh3P);
            mem.ExecuteReadScatter(hScatter);

            if (Utils::ValidPtr(weaponTrajectoryData))
            {
                // Removed the forced StopAiming logic that was blocking standard weapons
            }

            mem.AddScatterReadRequest(hScatter, weaponTrajectoryData + Offset::TrajectoryConfig, &g_currentWeaponData.TrajectoryConfig);
            mem.ExecuteReadScatter(hScatter);

            g_currentWeaponData.Mesh3P = Decrypt::Xe(g_currentWeaponData.Mesh3P);
            trajectoryConfig.BallisticCurve = g_currentWeaponData.GetTrajectoryConfig<uint64_t>(Offset::BallisticCurve);
            g_currentWeaponData.TrajectoryConfigs.InitialSpeed = g_currentWeaponData.GetTrajectoryConfig<float>(0);
            g_currentWeaponData.TrajectoryConfigs.SimulationSubstepTime = g_currentWeaponData.GetTrajectoryConfig<float>(0x40);
            g_currentWeaponData.TrajectoryConfigs.VDragCoefficient = g_currentWeaponData.GetTrajectoryConfig<float>(0x44);
            g_currentWeaponData.TrajectoryConfigs.BDS = g_currentWeaponData.GetTrajectoryConfig<float>(0x48);
            g_currentWeaponData.TrajectoryConfigs.ReferenceDistance = g_currentWeaponData.GetTrajectoryConfig<float>(0x34);

            if (g_currentWeaponData.TrajectoryConfigs.InitialSpeed < 100)
            {
                g_currentWeaponData.TrajectoryConfigs.InitialSpeed = 800;
            }

            mem.AddScatterReadRequest(hScatter, trajectoryConfig.BallisticCurve + Offset::FloatCurves, (FRichCurve*)&g_currentWeaponData.FloatCurves);
            mem.AddScatterReadRequest(hScatter, trajectoryConfig.BallisticCurve + Offset::FloatCurves + Offset::Keys, (FRichCurveKeyArray*)&g_currentWeaponData.RichCurveKeyArray);
            mem.ExecuteReadScatter(hScatter);

            std::vector<FRichCurveKey> keys(g_currentWeaponData.RichCurveKeyArray.Count);

            mem.Read(g_currentWeaponData.RichCurveKeyArray.Data, keys.data(), sizeof(FRichCurveKey) * g_currentWeaponData.RichCurveKeyArray.Count);
            g_currentWeaponData.RichCurveKeys = keys;

            g_lastCurrentWeapon = GameData.LocalPlayerInfo.CurrentWeapon;
        }
        return true;
    }

    static void Run()
    {
        auto hScatter = mem.CreateScatterHandle();

        while (true)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            // Xoa sleep 1ms de tranh nap chong cheo - chi sleep o cuoi vong lap hoac khi khong aim
            
            if (GameData.Scene != Scene::Gaming || GameData.bShowMouseCursor) {
                MouseDispatcher::Reset();
                // Ngoai tran dau hoac dang hien chuot: reset thoi gian AimThread ve 0 de tranh bi "dong bang" so cu
                GameData.Performance.AimThreadMs = 0.0f;
                Sleep(GameData.ThreadSleep);
                continue;
            }

            // 0. CAP NHAT THONG TIN VU KHI (Toi uu: Chi doc khi doi sung)
            UpdateWeaponInfo(hScatter);

            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::None || GameData.LocalPlayerInfo.CurrentWeapon == 0) {
                GameData.precision_calibration.Lock = false;
                continue;
            }

            // Lay config phím tu menu
            precision_calibrationConfig config = GameData.Config.precision_calibration.Configs[GameData.Config.precision_calibration.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];

            bool isAimKeyDown = GameData.Keyboard.IsKeyDown(config.First.Key) || GameData.Keyboard.IsKeyDown(config.Second.Key);

            if (!config.enable) {
                GameData.precision_calibration.Lock = false;
                continue;
            }

            if (!isAimKeyDown) {
                GameData.precision_calibration.Lock = false;
                MouseDispatcher::Reset();
                // Nghi 10ms (100Hz) khi khong aim de giam tai PCIe
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (GameData.precision_calibration.Target == 0) {
                if (GameData.precision_calibration.Lock) {
                    Utils::Log(1, "[AIM] Target lost, unlocking.");
                    GameData.precision_calibration.Lock = false;
                }
                continue;
            }

            // TOI UU: Dung Pointer de tranh copy struct Player nang
            Player* playerPtr = Data::GetPlayersItemPtr(GameData.precision_calibration.Target);
            if (!playerPtr || playerPtr->Entity == 0 || playerPtr->Health <= 0) {
                GameData.precision_calibration.Target = 0;
                GameData.precision_calibration.Lock = false;
                continue;
            }
            Player& player = *playerPtr;

            if (!GameData.precision_calibration.Lock) {
                Utils::Log(1, "[AIM] Locking onto target: %s", player.Name.c_str());
            }
            GameData.precision_calibration.Lock = true;

            // SỬ DỤNG XƯƠNG ĐÃ ĐƯỢC CHỌN TỰ ĐỘNG TỪ PLAYERS.H
            int aimBone = GameData.precision_calibration.Bone;
            if (aimBone <= 0) aimBone = 15; 

            // Kiểm tra vật cản nếu bật VisibleCheck
            if (config.VisibleCheck && !player.IsVisible) {
                GameData.precision_calibration.Lock = false;
                continue;
            }

            FVector targetPos = player.Skeleton->LocationBones[aimBone];

            // 4. LAY DU LIEU CAMERA DONG BO (Tu CameraCache 0xA30, 0xA10)
            CameraData camera = Data::GetCamera();
            FVector cameraLocation = camera.Location;
            FRotator currentRotation = camera.Rotation; // Sử dụng Camera Rotation làm gốc tham chiếu hiện tại
            float cameraFOV = camera.FOV <= 0.0f ? 90.0f : camera.FOV;

            // PREDICTION
            float initialSpeed = g_currentWeaponData.TrajectoryConfigs.InitialSpeed;
            float gravityZ = g_currentWeaponData.TrajectoryGravityZ;

            if (config.Prediction && initialSpeed > 100.0f) {
                FVector safeVelocity = player.Velocity;
                if (safeVelocity.Length() > 4000.0f) safeVelocity = {0,0,0};
                targetPos = GetAdvancedPrediction(cameraLocation, targetPos, safeVelocity, initialSpeed, gravityZ);
            }

            FRotator targetRotation = (targetPos - cameraLocation).GetDirectionRotator();
            FRotator deltaRotation = targetRotation - currentRotation;
            deltaRotation.Clamp();

            float mouseXSensitivity = 0.02f; 
            float mouseYSensitivity = 0.02f;
            float fovRatio = 90.0f / cameraFOV;

            // Áp dụng Tốc độ X và Tốc độ Y từ Menu (XSpeed, YSpeed)
            float moveX = (deltaRotation.Yaw / mouseXSensitivity) * fovRatio * (config.XSpeed / 100.0f);
            float moveY = (-deltaRotation.Pitch / mouseYSensitivity) * fovRatio * (config.YSpeed / 100.0f);



            // Bo Smooth: X Speed va Y Speed se tuong ung truc tiep voi toc do di tam
            float mortarFactor = g_isMortars ? 0.4f : 1.0f; // Mortar can cham hon de chinh xac (tuong duong smooth 2.5x cu)
            float finalMoveX = moveX * mortarFactor;
            float finalMoveY = moveY * mortarFactor;

            // Giới hạn bước di chuyển tối đa cao hơn để cho phép snap nhanh
            float maxStep = 100.0f; 
            finalMoveX = std::clamp(finalMoveX, -maxStep, maxStep);
            finalMoveY = std::clamp(finalMoveY, -maxStep, maxStep);

            // Deadzone cuc nho: Loc nhiễu tọa độ để tránh rung (Oscillation)
            constexpr float kDeadzone = 0.3f;
            if (std::abs(finalMoveX) < kDeadzone && std::abs(finalMoveY) < kDeadzone) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }

            if (finalMoveX != 0.0f || finalMoveY != 0.0f) {
                MouseDispatcher::AddAim(finalMoveX, finalMoveY);
            }

            // Luôn giữ Lock true khi đã qua các bước kiểm tra phím và data hợp lệ
            GameData.precision_calibration.Lock = true;

            // Dong bo voi toc do doc DMA (4-8ms) - Tang tan so de phan hoi nhanh hon
            std::this_thread::sleep_for(std::chrono::milliseconds(4));

            float distFromCenter = sqrt(moveX * moveX + moveY * moveY);
            float maxTriggerDist = config.banjiAimDistance > 0 ? (float)config.banjiAimDistance : 300.0f;

            if (config.AutomaticShooting && distFromCenter <= (float)config.AutomaticShootingFOV && player.Distance <= maxTriggerDist) {
                static uint64_t lastShot1 = 0;
                if (GetTickCount64() - lastShot1 > (uint64_t)config.AutomaticShootingTime) {
                    TriggerMouseLeft(30);
                    lastShot1 = GetTickCount64();
                }
            } 
            else if (config.AimAndShot && distFromCenter <= config.Threshold && player.Distance <= maxTriggerDist) {
                static uint64_t lastShot2 = 0;
                if (GetTickCount64() - lastShot2 > (uint64_t)config.Delay1) {
                    TriggerMouseLeft(30);
                    lastShot2 = GetTickCount64();
                }
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            GameData.Performance.AimThreadMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        }
        mem.CloseScatterHandle(hScatter);
    }
};
