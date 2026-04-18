---
name: pubg_offsets
description: Cập nhật địa chỉ bộ nhớ cho PUBG-2 sử dụng XML structure.
---

<skill_definition name="pubg_offsets">
    <objective>Cập nhật file cấu hình và giải mã khi game PUBG có phiên bản mới.</objective>

    <target_files>
        <file path=".shared/pubg_config.hpp" type="config" description="File chứa toàn bộ offset và keys." />
        <file path="pubg/sdk/offsets.hpp" type="mapping" description="File ánh xạ hệ thống." />
    </target_files>

    <update_procedure>
        <step id="1" category="CORE">Cập nhật UWorld, GNames, GObjects (XenuineDecrypt).</step>
        <section id="2" category="ENCRYPTION">
            <update>Cập nhật 16 HealthKey (0-15)</update>
            <update>Cập nhật logic trong hàm 'decrypt_cindex'</update>
        </section>
        <step id="3" category="VISP">Cập nhật Mesh, BoneArray, Camera offsets.</step>
    </update_procedure>

    <automation_rules>
        // turbo
        <action type="file_edit">Sử dụng công cụ thay đổi nội dung file để áp dụng thông số mới nhất.</action>
    </automation_rules>

    <maintenance_check>
        <verification>Kiểm tra 'version' string trong pubg_config.hpp.</verification>
        <verification>Build lại solution qua skill 'pubg_build' để xác nhận compile thành công.</verification>
    </maintenance_check>
</skill_definition>
