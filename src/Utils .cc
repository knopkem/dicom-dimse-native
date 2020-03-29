#include "Utils.h"

using json = nlohmann::json;

namespace ns {

      std::string createJsonResponse(eStatus status, const std::string& message, const json& j) 
    {
        std::string meaning = "success";
        if (status == PENDING) {
            meaning = "pending";
        }
        if (status == FAILURE) {
            meaning = "failure";
        }
        json v = json::object();
        v["container"] = j;
        v["messsage"] = message;
        v["code"] = (int)status;
        v["status"] = meaning;
        return v.dump();
    }

}