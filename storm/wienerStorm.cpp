#include "wienerStorm.hxx"
#include "storm_fit.h"
#define CSTACK_DEFNS
#define R_INTERFACE_PTRS
#define HAVE_UINTPTR_T
#include <cstdint>
#ifndef __WIN__
#include <Rinterface.h>
#endif
#include <R_ext/Parse.h>

std::mutex wienerStorm_R_mutex; // R is not thread-safe

void fitPSF(DataParams &params, MultiArray<2, double> &ps) {
    wienerStorm_R_mutex.lock();
    SEXP mat, fun, t;
    PROTECT(mat = Rf_allocMatrix(REALSXP, ps.shape(1), ps.shape(0)));
    std::memcpy(REAL(mat), ps.data(), ps.size() * sizeof(double));
    PROTECT(fun = t = Rf_allocList(2));
    SET_TYPEOF(fun, LANGSXP);
    SETCAR(t, Rf_install("fit.filter"));
    t = CDR(t);
    SETCAR(t, mat);

    PROTECT(t = Rf_eval(fun, R_GlobalEnv));
    double *sigmas = REAL(t);
    double sigmax = ps.shape(0) / (2 * std::sqrt(2) * M_PI * sigmas[0]);
    double sigmay = ps.shape(1) / (2 * std::sqrt(2) * M_PI * sigmas[1]);
    params.setSigma((sigmax + sigmay) / 2);

    UNPROTECT(3);
    R_gc();
    wienerStorm_R_mutex.unlock();
}

void preventRConsoleWrite(const char* buf, int buflen)
{}

bool initR(int argc, char **argv, bool withRestart)
{
    if (std::getenv("R_HOME") == nullptr) {
        if (!withRestart)
            return false;
        char **args = (char**)std::malloc((argc + 3) * sizeof(char*));
        args[0] = (char*)"R";
        args[1] = (char*)"CMD";
        for (int i = 0, j = 2; i < argc; ++i, ++j)
            args[j] = argv[i];
        args[argc + 2] = nullptr;
        int ret = execvp(args[0], args);
        /*std::string reason;
        switch (errno) {
            case ENOENT:
                reason << "ENOENT";
                break;
            case ENOTDIR:
                reason << "ENOTDIR";
                break;
            case E2BIG:
                reason << "E2BIG";
                break;
            case EACCES:
                reason << "EACCES";
                break;
            case EINVAL:
                reason << "EINVAL";
                break;
            case ELOOP:
                reason << "ELOOP";
                break;
            case ENOMEM:
                reason << "ENOMEM";
                break;
            case ETXTBSY:
                reason << "ETXTBSY";
                break;
            default:
                reason << "unknown";
                break;
        }*/
        std::free(args);
        return false;
    }
    char *Rargv[] = {(char*)"REmbeddedStorm", (char*)"--silent", (char*)"--no-save"};
#ifndef __WIN__
    R_SignalHandlers = FALSE;
#endif
    Rf_initEmbeddedR(sizeof(Rargv) / sizeof(Rargv[0]), Rargv);
#ifndef __WIN__
    R_CStackLimit = (uintptr_t)-1;
#endif

    SEXP fun, t, tmp;
    PROTECT(tmp = Rf_ScalarInteger(42));

    PROTECT(fun = t = Rf_allocList(2));
    SET_TYPEOF(fun, LANGSXP);
    SETCAR(t, Rf_install("set.seed"));
    t = CDR(t);
    SETCAR(t, tmp);
    Rf_eval(fun, R_GlobalEnv);
    UNPROTECT(2);

    ParseStatus status;
    PROTECT(tmp = Rf_mkString(storm_fit));
    PROTECT(t = R_ParseVector(tmp, -1, &status, R_NilValue));

    if (status != PARSE_OK) {
        UNPROTECT(2);
        return false;
    }
    for (R_len_t i = 0; i < Rf_length(t); ++i) {
        Rf_eval(VECTOR_ELT(t, i), R_GlobalEnv);
    }
    UNPROTECT(2);
    return true;
}

void endR()
{
    // prevent printing of R warnings
#ifndef __WIN__
    void (*ptr_R_WriteConsole_old)(const char *, int) = ptr_R_WriteConsole;
    FILE *R_Consolefile_old = R_Consolefile;
    ptr_R_WriteConsole = preventRConsoleWrite;
    R_Consolefile = NULL;
#endif
    Rf_endEmbeddedR(0);
#ifndef __WIN__
    ptr_R_WriteConsole = ptr_R_WriteConsole_old;
    R_Consolefile = R_Consolefile_old;
#endif
}

