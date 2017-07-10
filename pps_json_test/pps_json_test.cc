#include <stdlib.h>
#include <errno.h>
#include <sys/pps.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define CMD_HID             "hid"
#define CMD_SIRI            "siri"
#define CMD_NIGHT           "night"
#define CMD_CHANGEMODES     "changemodes"
#define CMD_HID_TOUCHPAD    "touchpad"

#define kSourceType         "resource"
#define kSourceAduio        "mainaudio"
#define kSourceScreen   "mainscreen"

#define kTransferType                                          "transferType"
#define kTransferTypeString_NotApplicable       "n/a"
#define kTransferTypeString_Take                    "take"
#define kTransferTypeString_Untake              "untake"
#define kTransferTypeString_Borrow              "borrow"
#define kTransferTypeString_Unborrow            "unborrow"

#define kTransferPriority                                             "transferPriority"
#define kTransferPriorityString_NotApplicable       "n/a"
#define kTransferPriorityString_NiceToHave          "niceToHave"
#define kTransferPriorityString_UserInitiated       "userInitiated"

#define kTakeConstraint                                 "takeConstraint"
#define kBorrowConstraint                               "borrowConstraint"
#define kConstraintString_NotApplicable     "n/a"
#define kConstraintString_Anytime           "anytime"
#define kConstraintString_UserInitiated     "userInitiated"
#define kConstraintString_Never             "never"
static const char STR_PPS_CTRL_OBJ_ATTRIBUTES[]         = "?server,wait";
static const char STR_PPS_DIR[]                         = "/pps/services/multimedia/qnxcarplay";
static const char STR_PPS_OBJ_CTRL[]                    = "controltest";

static const char *const cmd_attrs[] = {
    CMD_HID,
    CMD_SIRI,
    CMD_NIGHT,
    CMD_CHANGEMODES,
    CMD_HID_TOUCHPAD,
    NULL
};
static const char *const cmd_objs[] = { NULL };

int main(int argc, char *argv[]) {
    int rc;
    char *file;

    rc = mkdir(STR_PPS_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (rc == -1 && errno != EEXIST)
    {
        return 0;
    }
    file = (char *)alloca(strlen(STR_PPS_DIR) + strlen(STR_PPS_OBJ_CTRL) + 1 + sizeof(STR_PPS_CTRL_OBJ_ATTRIBUTES));
    if (file == NULL)
    {
        return 0;
    }

    sprintf(file, "%s/%s%s", STR_PPS_DIR, STR_PPS_OBJ_CTRL, STR_PPS_CTRL_OBJ_ATTRIBUTES);
    int ctrlfd = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (ctrlfd == -1)
    {
        return 0;
    }

    int n;
    char buffer[2048];

    while (1) {
        n = read(ctrlfd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            pps_attrib_t    info;
            pps_status_t    rc;

            memset(&info, 0, sizeof(info));
            char *line = buffer;
            fprintf(stderr, "line is %s n is %d\n", buffer, n);
            while ((rc = ppsparse(&line, cmd_objs, cmd_attrs, &info, 0)) != PPS_END) {
               if (rc == PPS_ATTRIBUTE) {
                   int rec = 0;
                   pps_decoder_t decoder;
                   const char *str;

                   rec = pps_decoder_initialize(&decoder, NULL);
                   if (rec){
                       fprintf(stderr, "pps_decoder_initialize fail\n");
                   }

                   fprintf(stderr, "ppsStr is %s\n", info.value);
                   rec = pps_decoder_parse_json_str(&decoder, (char *)info.value);
                   if (rec){
                       fprintf(stderr, "Cannot parse PPS decoder for JSON ChangeMode obj\n");
                   }

                   pps_decoder_push(&decoder, NULL);

                   str = NULL;
                   pps_decoder_get_string(&decoder, kSourceType, &str);
                   fprintf(stderr, "source type is %s\n", str);

                   str = NULL;
                    pps_decoder_get_string(&decoder, kTransferType, &str);
                    fprintf(stderr, " kTransferType is %s  %d\n", str, __LINE__);

                    str = NULL;
                    pps_decoder_get_string(&decoder, kTransferPriority, &str);
                    fprintf(stderr, " kTransferPriority is %s  %d\n", str, __LINE__);

                    str = NULL;
                    pps_decoder_get_string(&decoder, kTakeConstraint, &str);
                    fprintf(stderr, " kTakeConstraint is %s  %d\n", str, __LINE__);

                    str = NULL;
                    pps_decoder_get_string(&decoder, kBorrowConstraint, &str);
                    fprintf(stderr, " kBorrowConstraint is %s  %d\n", str, __LINE__);
               } else if (rc == PPS_ERROR) {
                   break;
               }
            }
        }
    }
}
