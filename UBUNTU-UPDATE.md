# Ubuntu 20.04 Retirement Update

## 🔄 Changes Made

Due to the scheduled Ubuntu 20.04 retirement on **2025-04-15**, the following CI/CD configuration files have been updated:

### Updated Files

1. **`.github/workflows/release-simple.yml`**
   - Changed from `ubuntu-20.04` to `ubuntu-22.04`
   - Ensures continued build support after retirement

2. **`.github/workflows/ci.yml`**
   - Changed from `ubuntu-20.04` to `ubuntu-22.04`
   - Maintains CI testing capabilities

### Files Already Compatible

1. **`.github/workflows/release.yml`**
   - Already uses `ubuntu-latest` (automatically selects latest supported version)
   - No changes needed

2. **`CI-README.md`**
   - Already references "Ubuntu latest"
   - No changes needed

## 📅 Timeline

- **Previous**: Ubuntu 20.04 LTS
- **Retirement Date**: 2025-04-15
- **New Version**: Ubuntu 22.04 LTS
- **Action Taken**: 2025-06-01 (proactive update)

## ✅ Impact

- ✅ All CI/CD workflows will continue working after Ubuntu 20.04 retirement
- ✅ Build environment remains stable and secure
- ✅ No changes required to build scripts or source code
- ✅ Binary compatibility maintained

## 🔗 Reference

For more details about the Ubuntu 20.04 retirement, see:
https://github.com/actions/runner-images/issues/11101

---

**Status**: ✅ TCPING CI/CD is now fully prepared for Ubuntu 20.04 retirement
