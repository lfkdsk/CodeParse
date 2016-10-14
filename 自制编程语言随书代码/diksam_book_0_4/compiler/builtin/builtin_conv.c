#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DVM.h"
#include "DBG.h"

#define ROOT_DIR ("../../require/")
#define LINE_BUF_SIZE (1024)
typedef struct {
    char *package_name;
    char *suffix;
} TargetPackage;

static TargetPackage st_target_package[] = {
    {"diksam.lang", "dkh"},
    {"diksam.lang", "dkm"},
    {"diksam.math", "dkh"}
};

static void
dump_file(char *file_name, FILE *out_fp)
{
    FILE *fp;
    char buf[LINE_BUF_SIZE];

    fp = fopen(file_name, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", file_name);
        exit(1);
    }
    while (fgets(buf, LINE_BUF_SIZE, fp) != NULL) {
        fputs(buf, out_fp);
    }
    fclose(fp);
}

static void
conv_period(char *src, char *dest, int to_char)
{
    int i;

    for (i = 0; src[i] != '\0'; i++) {
        if (src[i] == '.') {
            dest[i] = to_char;
        } else {
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';
}

static void conv_str(char *src, char *dest)
{
    int src_idx;
    int dest_idx;

    for (src_idx = 0, dest_idx = 0; src[src_idx] != '\0'; src_idx++) {
        if (src[src_idx] == '\"') {
            dest[dest_idx++] = '\\';
            dest[dest_idx++] = '\"';
        } else if (src[src_idx] == '\\') {
            dest[dest_idx++] = '\\';
            dest[dest_idx++] = '\\';
        } else if (src[src_idx] == '\t') {
            dest[dest_idx++] = '\\';
            dest[dest_idx++] = 't';
        } else if (src[src_idx] == '\n') {
            dest[dest_idx++] = '\0';
        } else {
            dest[dest_idx++] = src[src_idx];
        }
    }
    dest[dest_idx] = '\0';
}

static void
do_conv(TargetPackage *target, FILE *out_fp)
{
    char file_path[LINE_BUF_SIZE];
    char package_path[LINE_BUF_SIZE];
    char variable_name[LINE_BUF_SIZE];
    FILE *in_fp;
    char line[LINE_BUF_SIZE];
    char conv_line[LINE_BUF_SIZE];

    conv_period(target->package_name, package_path, '/');
    sprintf(file_path, "%s%s.%s", ROOT_DIR, package_path, target->suffix);

    in_fp = fopen(file_path, "r");
    if (in_fp == NULL) {
        fprintf(stderr, "%s not found.\n", file_path);
        exit(1);
    }
    conv_period(target->package_name, variable_name, '_');

    fprintf(out_fp, "char *st_%s_%s_text[] = {\n", variable_name,
            target->suffix);
    while (fgets(line, LINE_BUF_SIZE, in_fp) != NULL) {
        conv_str(line, conv_line);
        fprintf(out_fp, "    \"%s\\n\",\n", conv_line);
    }
    fprintf(out_fp, "    NULL\n");
    fprintf(out_fp, "};\n");
}

int
main(int argc, char **argv)
{
    int i;
    FILE *out_fp;
    char variable_name[LINE_BUF_SIZE];

    out_fp = fopen("builtin.c", "w");
    if (out_fp == NULL) {
        fprintf(stderr, "builtin.c could not be opened.\n");
        exit(1);
    }
    dump_file("head.txt", out_fp);
    
    for (i = 0; i < sizeof(st_target_package) / sizeof(TargetPackage); i++) {
        do_conv(&st_target_package[i], out_fp);
    }
    fprintf(out_fp, "\nBuiltinScript dkc_builtin_script[] = {\n");
    for (i = 0; i < sizeof(st_target_package) / sizeof(TargetPackage); i++) {
        char *suffix;
        conv_period(st_target_package[i].package_name, variable_name, '_');
        if (!strcmp(st_target_package[i].suffix, "dkh")) {
            suffix = "DKH_SOURCE";
        } else {
            DBG_assert(!strcmp(st_target_package[i].suffix, "dkm"),
                       ("suffix..%s\n", st_target_package[i].suffix));
            suffix = "DKM_SOURCE";
        }
        fprintf(out_fp, "    {\"%s\", %s, st_%s_%s_text},\n",
                st_target_package[i].package_name,
                suffix,
                variable_name,
                st_target_package[i].suffix);
    }
    fprintf(out_fp, "    {NULL, DKM_SOURCE, NULL}\n");
    fprintf(out_fp, "};\n");
    dump_file("tail.txt", out_fp);

    fclose(out_fp);

    return 0;
}
