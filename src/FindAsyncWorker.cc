#include "FindAsyncWorker.h"

#include "dcmtk/config/osconfig.h"      /* make sure OS specific configuration is included first */

#include "dcmtk/dcmnet/dfindscu.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcostrmz.h"     /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmtls/tlsopt.h"        /* for DcmTLSOptions */

#ifdef WITH_ZLIB
#include <zlib.h>                       /* for zlibVersion() */
#endif
#ifdef DCMTK_ENABLE_CHARSET_CONVERSION
#include "dcmtk/ofstd/ofchrenc.h"       /* for OFCharacterEncoding */
#endif

#define OFFIS_CONSOLE_APPLICATION "findscu"

static OFLogger findscuLogger = OFLog::getLogger("dcmtk.apps." OFFIS_CONSOLE_APPLICATION);

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

/* default application titles */
#define APPLICATIONTITLE        "FINDSCU"
#define PEERAPPLICATIONTITLE    "ANY-SCP"

#define SHORTCOL 4
#define LONGCOL 20

#include "json.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <list>
#include <iomanip>

using json = nlohmann::json;

namespace {

    std::string str_toupper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), 
                    [](unsigned char c){ return std::toupper(c); }
                    );
        return s;
    }

    std::string int_to_hex( uint16_t i )
    {
    std::stringstream stream;
    stream  << std::setfill ('0') << std::setw(sizeof(uint16_t)*2) 
            << std::hex << i;
    return str_toupper(stream.str());
    }
}

FindAsyncWorker::FindAsyncWorker(std::string data, Function &callback) : BaseAsyncWorker(data, callback)
{
}

void FindAsyncWorker::Execute(const ExecutionProgress& progress)
{
    ns::sInput in = ns::parseInputJson(_input);

    if (in.tags.empty()) {
        SetErrorJson("Tags not set");
        return;
    }

    if (!in.source.valid()) {
        SetErrorJson("Source not set");
        return;
    }

    if (!in.target.valid()) {
        SetErrorJson("Target not set");
        return;
    }

}
