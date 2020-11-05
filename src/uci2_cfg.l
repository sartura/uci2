%{
#include <stdio.h>
#include "uci2_parser.h"
char* uci_unq(char* str, int l);
char* uci_regex_unq(char* str, int l);
%}

%option noyywrap reentrant bison-bridge
%option outfile="uci2_lexer.c" header-file="uci2_lexer.h"

newline              \n
ws                   [ \t]
comment              "#".*
value                ('[^'\n\t]*')|(\"[^\"\n\t]*\")|([^ '\"\n\t]+)

config               "config"
option               "option"
package              "package"
list                 "list"
type_dnsmasq         "dnsmasq"
type_dhcp            "dhcp"
type_dropbear        "dropbear"
type_rule            "rule"
type_rule6           "rule6"
type_redirect        "redirect"
type_zone            "zone"
type_forwarding      "forwarding"
type_interface       "interface"
type_route           "route"
type_route6          "route6"
type_droute          "disabled_route"
type_switch          "switch"
type_switch_vlan     "switch_vlan"
type_switch_port     "switch_vlan"
type_system          "system"
type_wifi_dev        "wifi-device"
type_wifi_iface      "wifi-iface"
type_general         "general"
type_defaults        "defaults"
type_bbstored        "bbstored"
type_raidfile        "raidfile"
type_service         "service"
type_etherwake       "etherwake"
type_target          "target"
type_mount           "mount"
type_swap            "swap"
type_hd_idle         "hd-idle"
type_httpd           "httpd"
type_mini_snmpd      "mini_snmpd"
type_mountd          "mountd"
type_member          "member"
type_policy          "policy"
type_instance        "instance"
type_ntpclient       "ntpclient"
type_ntpdrift        "ntpdrift"
type_ntpserever      "ntpserver"
type_pureftpd        "pure-ftpd"
type_racoon          "racoon"
type_tunnel          "tunnel"
type_p1_proposal     "p1_proposal"
type_sainfo          "sainfo"
type_p2_proposal     "p2_proposal"
type_certificate     "certificate"
type_samba           "samba"
type_sambashare      "sambashare"
type_exec            "exec"
type_queue           "queue"
type_server          "server"
type_tunnelR         "tunnelR"
type_tunnelL         "tunnelL"
type_stund           "stund"
type_transmission    "transmission"
type_uhttpd          "uhttpd"
type_upnpd           "upnpd"
type_perm_rule       "perm_rule"
type_user            "user"
type_ushare          "ushare"
type_vblade          "vblade"
type_vnstat          "vnstat"
type_wifitoggle      "wifitoggle"
type_wol_target      "wol-target"
type_znc             "znc"
type_timeserver      "timeserver"

%s ST_VALUE


%%
{newline}+          { BEGIN(INITIAL); }
{comment}           ; 
{ws}*               ;
{option}            { BEGIN(ST_VALUE); return OPTION; }
{list}              { BEGIN(ST_VALUE); return LIST; }
<ST_VALUE>{value}   { yylval->str = uci_unq(yytext, yyleng); return VALUE; }
{config}            { BEGIN(ST_VALUE); return CONFIG; }
{package}           { BEGIN(ST_VALUE); return PACKAGE; }

.                   { return 1; }
%%

// How Flex Handles Ambiguous Patterns (config and value)
// * Match the longest possible string every time the scanner matches input
// * In the case of a tie, use the pattern that appears first in the program

// basic unquote method
char* uci_unq(char* str, int l){
    // if not quoted, just duplicate
    if(l < 2) return strdup(str);
    // if no starting quotes, duplicate
    if (!(str[0] == 0x27 || str[0] == 0x22)) return strdup(str);
    // i no end quotes, duplicate
    if (!(str[l - 1] == 0x27 || str[l - 1] == 0x22)) return strdup(str);
    // l - 2 quotes + 1 NULL
    char* res = malloc(l - 1);
    // skip first and last quotes and NULL
    memcpy(res, str + 1, l - 2);
    // NULL
    res[l - 2] = 0;
    return res;
}

// int to str
char* uci_itos(int num){
#ifdef _GNU_SOURCE
    char* res;
    int s = asprintf(&res, "%d", num);
    if(s > 0) return res;
    return 0;
#else
    int l = snprintf(NULL, 0, "%d", num);
    char* res = malloc(l + 1);
    if(!res) return 0;
    snprintf(res, l + 1, "%d", num);
    return res;
#endif
}

// yyerror
void yyerror(yyscan_t scanner, uci2_parser_ctx_t* ctx, const char* str){
    // no error output
    // printf("%s\n", str);
}
