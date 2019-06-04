#pragma once 

#include "radx_core.hpp"
#include "radx_device.hpp"

namespace radx {
    namespace paths {
        static inline const std::array<std::string, 6> pathNames{ "universal", "amdgpu", "nvidia", "intel", "vega10", "turing" };

        static inline constexpr const auto permutation = "radix/permutation.comp";
        static inline constexpr const auto partition = "radix/partition.comp";
        static inline constexpr const auto counting = "radix/counting.comp";
        static inline constexpr const auto scattering = "radix/scattering.comp";
        static inline constexpr const auto indiction = "radix/indiction.comp";


        class Generation {
        protected:
            std::string mod = "";
        public:
            Generation(const std::string& mod = ""): mod(mod) {}
            operator const std::string&() const {
                return mod;
            }
        };


        class NVLegacy : public Generation { public: NVLegacy(const std::string& mod = "nvidia") { this->mod = mod; }; };
        class NVTuring : public Generation { public: NVTuring(const std::string& mod = "turing") { this->mod = mod; }; };

        class AMDLegacy : public Generation { public: AMDLegacy(const std::string& mod = "amdgpu") { this->mod = mod; }; };
        class AMDVega10 : public Generation { public: AMDVega10(const std::string& mod = "vega10") { this->mod = mod; }; };


        class UniversalType {
        protected:
            std::string pathname = "universal";
        public:
            UniversalType(const std::string& pathname = "universal") : pathname(pathname) {};
            operator const std::string&() const { return pathname; };
        };

        template<class M = NVLegacy>
        class NVIDIA : public UniversalType {
        public:
            NVIDIA(const std::string& pathname = "") { this->pathname = pathname + (const std::string&)M(); };
        };

        template<class M = AMDLegacy>
        class AMD : public UniversalType {
        public:
            AMD(const std::string& pathname = "") { this->pathname = pathname + (const std::string&)M(); };
        };

        class Intel : public UniversalType {
        public:
            Intel(const std::string& pathname = "intel") { this->pathname = pathname; };
        };



        class DriverWrapBase {
        protected: 
            std::shared_ptr<UniversalType> driverType = {};
            std::string directory = "";
        public:
            DriverWrapBase(const std::string& directory = "./intrusive") : directory(directory) {};
            std::string getPath(const std::string& fpath) const {
                return (directory + "/" + std::string(*driverType) + "/" + fpath + ".spv");
            }
            std::string getDriverName() const {
                return std::string(*driverType);
            }
        };

        template<class T = UniversalType>
        class DriverWrap: public DriverWrapBase {
        public:
            DriverWrap(const std::string& directory = "./intrusive") {
                this->directory = directory;
                this->driverType = std::static_pointer_cast<UniversalType>(std::make_shared<T>());
            }
        };

        inline std::shared_ptr<DriverWrapBase> getNamedDriver(const uint32_t& vendorID, const int modifier = 0, const std::string& directory = "./intrusive") {
            switch (vendorID) {
            case 4318:
                if (modifier) {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<NVIDIA<NVTuring>>>(directory));
                }
                else {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<NVIDIA<NVLegacy>>>(directory));
                }
                break;
            case 4098:
                if (modifier) {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<AMD<AMDVega10>>>(directory));
                }
                else {
                    return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<AMD<AMDLegacy>>>(directory));
                }
                break;
            case 32902:
                return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<Intel>>(directory));
                break;
            default:
                return std::dynamic_pointer_cast<DriverWrapBase>(std::make_shared<DriverWrap<UniversalType>>(directory));
            };
        };
    };
};
