---
name: pubg_build
description: Build solution phao_final.sln cho PUBG-2 sử dụng XML structure.
---

<skill_definition name="pubg_build">
    <objective>Thực hiện biên dịch và đóng gói giải pháp PUBG-2 (phao_final.sln) sang định dạng thực thi x64 Release.</objective>
    
    <environment_checks>
        <check type="mandatory">Đảm bảo 'msbuild' đã có trong PATH hệ thống (Windows Environment Variables).</check>
        <check type="path_verification">Xác định file 'phao_final.sln' tồn tại ở thư mục gốc.</check>
    </environment_checks>

    <execution_logic>
        <step id="1" description="Prepare Environment">Kiểm tra các thư viện liên kết (.lib) như vmm.lib và DMALibrary.lib.</step>
        <step id="2" description="Compile Solution">Sử dụng MSBuild để xây dựng với cấu hình Release | x64.</step>
        <step id="3" description="Verify Binary">Xác nhận file thực thi (.exe) đã được tạo trong thư mục /bin/Release/.</step>
    </execution_logic>

    <automation_rules>
        // turbo
        <command_script type="powershell" context="safe_to_run">
            msbuild /p:Configuration=Release /p:Platform=x64 phao_final.sln
        </command_script>
    </automation_rules>

    <error_handling>
        <on_fail>Nếu lỗi link thư viện, kiểm tra lại đường dẫn .shared và DMALibrary headers.</on_fail>
    </error_handling>
</skill_definition>
