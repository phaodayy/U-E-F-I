#define NOMINMAX
#include "win_runtime_hardening.h"

#include <Windows.h>

namespace protec
{
    namespace
    {
        void update_last_error(hardening_result_t& result)
        {
            result.last_error = GetLastError();
        }
    }

    hardening_result_t apply_baseline_hardening(const hardening_options_t& options)
    {
        hardening_result_t result = {};

        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

        if (options.enable_heap_termination)
        {
            if (HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0))
            {
                result.heap_termination_enabled = true;
            }
            else
            {
                update_last_error(result);
            }
        }

        if (options.harden_dll_search_order)
        {
            if (SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS))
            {
                result.dll_search_order_hardened = true;
            }
            else if (result.last_error == ERROR_SUCCESS)
            {
                update_last_error(result);
            }
        }

        if (options.disable_extension_points)
        {
            PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY policy = {};
            policy.DisableExtensionPoints = 1;

            if (SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &policy, sizeof(policy)))
            {
                result.extension_points_disabled = true;
            }
            else if (result.last_error == ERROR_SUCCESS)
            {
                update_last_error(result);
            }
        }

        if (options.block_remote_images || options.block_low_integrity_images || options.prefer_system32_images)
        {
            PROCESS_MITIGATION_IMAGE_LOAD_POLICY policy = {};
            policy.NoRemoteImages = options.block_remote_images ? 1u : 0u;
            policy.NoLowMandatoryLabelImages = options.block_low_integrity_images ? 1u : 0u;
            policy.PreferSystem32Images = options.prefer_system32_images ? 1u : 0u;

            if (SetProcessMitigationPolicy(ProcessImageLoadPolicy, &policy, sizeof(policy)))
            {
                result.image_load_policy_applied = true;
            }
            else if (result.last_error == ERROR_SUCCESS)
            {
                update_last_error(result);
            }
        }

        if (options.disable_dynamic_code)
        {
            PROCESS_MITIGATION_DYNAMIC_CODE_POLICY policy = {};
            policy.ProhibitDynamicCode = 1;

            if (SetProcessMitigationPolicy(ProcessDynamicCodePolicy, &policy, sizeof(policy)))
            {
                result.dynamic_code_policy_applied = true;
            }
            else if (result.last_error == ERROR_SUCCESS)
            {
                update_last_error(result);
            }
        }

        return result;
    }

    bool is_debugger_likely_present()
    {
        if (IsDebuggerPresent())
        {
            return true;
        }

        BOOL remote_debugger = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote_debugger);
        return remote_debugger == TRUE;
    }

    void secure_wipe(void* const buffer, const SIZE_T size)
    {
        if (buffer == nullptr || size == 0)
        {
            return;
        }

        SecureZeroMemory(buffer, size);
    }
}
