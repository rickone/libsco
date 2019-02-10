#include "asyn_panic.h"
#include <cstdlib>
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>

void asyn::panic(const std::string& what) {
    size_t size = 1024;
    void** buffer = (void**)malloc(size * sizeof(void*));
    int bt_num = backtrace(buffer, size);

    char** bt_sym = backtrace_symbols(buffer, bt_num);
    if (bt_sym == nullptr) {
        free(buffer);
        return;
    }

    std::string info("panic: '");
    info.append(what);
    info.append("' backtrace:");

    for (int i = 0; i < bt_num; ++i) {
        char* name_begin = 0;
        char* name_end = 0;
        
        for (char* p = bt_sym[i]; *p; p++) {
            if (*p == '(') {
                name_begin = p;
            }           
            else if (*p == '+' && name_begin) {
                name_end = p;
                break;
            }
        }

        info.append("\n  #");
        info.append(std::to_string(i));
        info.append(" ");
        if (name_begin && name_end) {
            *name_begin++ = 0;
            *name_end++ = 0;
            int status = 0;
            char* real_name = abi::__cxa_demangle(name_begin, 0, 0, &status);
            std::string func;
            func.append(bt_sym[i]);
            func.append(": ");
            if (status == 0) {
                func.append(real_name);
                func.append(" +(");
                func.append(name_end);
            } else {
                if (*name_begin != 0) {
                    func.append(name_begin);
                    func.append("() +(");
                } else {
                    func.append(" +(");
                }
            }
            func.append(name_end);
            info.append(func);

            free(real_name);
        }
        else {
            info.append(bt_sym[i]);
        }
    }

    free(buffer);
    free(bt_sym);

    throw std::runtime_error(info);
}
