#include "Utils.h"

namespace ns {

    std::string tagVrName(imebra::tagVR_t tagVr) {

        switch(tagVr)
            {
            case imebra::tagVR_t::AE:
                return "AE";

            case imebra::tagVR_t::AS:
                return "AS";

            case imebra::tagVR_t::CS:
                return "CS";

            case imebra::tagVR_t::DS:
                return "DS";

            case imebra::tagVR_t::IS:
                return "IS";

            case imebra::tagVR_t::UR:
                return "UR";

            case imebra::tagVR_t::LO:
                return "LO";

            case imebra::tagVR_t::LT:
                return "LT";

            case imebra::tagVR_t::PN:
                return "PN";

            case imebra::tagVR_t::SH:
                return "SH";

            case imebra::tagVR_t::ST:
                return "ST";

            case imebra::tagVR_t::UC:
                return "UC";

            case imebra::tagVR_t::UI:
                return "UI";

            case imebra::tagVR_t::UT:
                return "UT";

            case imebra::tagVR_t::OB:
                return "OB";

            case imebra::tagVR_t::OL:
                return "OL";

            case imebra::tagVR_t::SB:
                return "SB";

            case imebra::tagVR_t::UN:
                return "UN";

            case imebra::tagVR_t::OW:
                return "OW";

            case imebra::tagVR_t::AT:
                return "AT";

            case imebra::tagVR_t::FL:
                return "FL";

            case imebra::tagVR_t::OF:
                return "OF";

            case imebra::tagVR_t::FD:
                return "DF";

            case imebra::tagVR_t::OD:
                return "OD";

            case imebra::tagVR_t::SL:
                return "SL";

            case imebra::tagVR_t::SS:
                return "SS";

            case imebra::tagVR_t::UL:
                return "UL";

            case imebra::tagVR_t::US:
                return "US";

            case imebra::tagVR_t::DA:
                return "DA";

            case imebra::tagVR_t::DT:
                return "DT";

            case imebra::tagVR_t::TM:
                return "TM";

            case imebra::tagVR_t::SQ:
                return "SQ";
            default:
                return "UN";
            }
            return "UN";
    }
}