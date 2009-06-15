/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */



/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        lsa.h
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        Public Client API
 *
 * Authors: Krishna Ganugapati (krishnag@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */
#ifndef __LSA_H__
#define __LSA_H__

/**
 * @file lsa.h
 * @brief LSASS Public Client API
 */

/**
 * @defgroup public Public API
 *
 */

/**
 * @defgroup connection Connections
 * @ingroup public
 */

/**
 * @defgroup user Users
 * @ingroup public
 */

/**
 * @defgroup group Groups
 * @ingroup public
 */

/**
 * @defgroup artifacts Artifacts
 * @ingroup public
 */

/**
 * @defgroup utility Utility
 * @ingroup public
 */

#include <lw/types.h>
#include <lw/attrs.h>

#ifndef LSA_ERRORS_DEFINED

#define LSA_ERRORS_DEFINED 1

/** Success */
#define LSA_ERROR_SUCCESS                                   0x0000
#define LSA_ERROR_INVALID_CACHE_PATH                        0x8000 // 32768
#define LSA_ERROR_INVALID_CONFIG_PATH                       0x8001 // 32769
#define LSA_ERROR_INVALID_PREFIX_PATH                       0x8002 // 32770
#define LSA_ERROR_INSUFFICIENT_BUFFER                       0x8003 // 32771
#define LSA_ERROR_OUT_OF_MEMORY                             0x8004 // 32772
#define LSA_ERROR_INVALID_MESSAGE                           0x8005 // 32773
#define LSA_ERROR_UNEXPECTED_MESSAGE                        0x8006 // 32774
#define LSA_ERROR_NO_SUCH_USER                              0x8007 // 32775
#define LSA_ERROR_DATA_ERROR                                0x8008 // 32776
#define LSA_ERROR_NOT_IMPLEMENTED                           0x8009 // 32777
#define LSA_ERROR_NO_CONTEXT_ITEM                           0x800A // 32778
#define LSA_ERROR_NO_SUCH_GROUP                             0x800B // 32779
#define LSA_ERROR_REGEX_COMPILE_FAILED                      0x800C // 32780
#define LSA_ERROR_NSS_EDIT_FAILED                           0x800D // 32781
#define LSA_ERROR_NO_HANDLER                                0x800E // 32782
#define LSA_ERROR_INTERNAL                                  0x800F // 32783
#define LSA_ERROR_NOT_HANDLED                               0x8010 // 32784
#define LSA_ERROR_INVALID_DNS_RESPONSE                      0x8011 // 32785
#define LSA_ERROR_DNS_RESOLUTION_FAILED                     0x8012 // 32786
#define LSA_ERROR_FAILED_TIME_CONVERSION                    0x8013 // 32787
#define LSA_ERROR_INVALID_SID                               0x8014 // 32788
#define LSA_ERROR_PASSWORD_MISMATCH                         0x8015 // 32789
#define LSA_ERROR_UNEXPECTED_DB_RESULT                      0x8016 // 32790
#define LSA_ERROR_PASSWORD_EXPIRED                          0x8017 // 32791
#define LSA_ERROR_ACCOUNT_EXPIRED                           0x8018 // 32792
#define LSA_ERROR_USER_EXISTS                               0x8019 // 32793
#define LSA_ERROR_GROUP_EXISTS                              0x801A // 32794
#define LSA_ERROR_INVALID_GROUP_INFO_LEVEL                  0x801B // 32795
#define LSA_ERROR_INVALID_USER_INFO_LEVEL                   0x801C // 32796
#define LSA_ERROR_UNSUPPORTED_USER_LEVEL                    0x801D // 32797
#define LSA_ERROR_UNSUPPORTED_GROUP_LEVEL                   0x801E // 32798
#define LSA_ERROR_INVALID_LOGIN_ID                          0x801F // 32799
#define LSA_ERROR_INVALID_HOMEDIR                           0x8020 // 32800
#define LSA_ERROR_INVALID_GROUP_NAME                        0x8021 // 32801
#define LSA_ERROR_NO_MORE_GROUPS                            0x8022 // 32802
#define LSA_ERROR_NO_MORE_USERS                             0x8023 // 32803
#define LSA_ERROR_FAILED_ADD_USER                           0x8024 // 32804
#define LSA_ERROR_FAILED_ADD_GROUP                          0x8025 // 32805
#define LSA_ERROR_INVALID_LSA_CONNECTION                    0x8026 // 32806
#define LSA_ERROR_INVALID_AUTH_PROVIDER                     0x8027 // 32807
#define LSA_ERROR_INVALID_PARAMETER                         0x8028 // 32808
#define LSA_ERROR_LDAP_NO_PARENT_DN                         0x8029 // 32809
#define LSA_ERROR_LDAP_ERROR                                0x802A // 32810
#define LSA_ERROR_NO_SUCH_DOMAIN                            0x802B // 32811
#define LSA_ERROR_LDAP_FAILED_GETDN                         0x802C // 32812
#define LSA_ERROR_DUPLICATE_DOMAINNAME                      0x802D // 32813
#define LSA_ERROR_KRB5_CALL_FAILED                          0x802E // 32814
#define LSA_ERROR_GSS_CALL_FAILED                           0x802F // 32815
#define LSA_ERROR_FAILED_FIND_DC                            0x8030 // 32816
#define LSA_ERROR_NO_SUCH_CELL                              0x8031 // 32817
#define LSA_ERROR_GROUP_IN_USE                              0x8032 // 32818
#define LSA_ERROR_FAILED_CREATE_HOMEDIR                     0x8033 // 32819
#define LSA_ERROR_PASSWORD_TOO_WEAK                         0x8034 // 32820
#define LSA_ERROR_INVALID_SID_REVISION                      0x8035 // 32821
#define LSA_ERROR_ACCOUNT_LOCKED                            0x8036 // 32822
#define LSA_ERROR_ACCOUNT_DISABLED                          0x8037 // 32823
#define LSA_ERROR_USER_CANNOT_CHANGE_PASSWD                 0x8038 // 32824
#define LSA_ERROR_LOAD_LIBRARY_FAILED                       0x8039 // 32825
#define LSA_ERROR_LOOKUP_SYMBOL_FAILED                      0x803A // 32826
#define LSA_ERROR_INVALID_EVENTLOG                          0x803B // 32827
#define LSA_ERROR_INVALID_CONFIG                            0x803C // 32828
#define LSA_ERROR_UNEXPECTED_TOKEN                          0x803D // 32829
#define LSA_ERROR_LDAP_NO_RECORDS_FOUND                     0x803E // 32830
#define LSA_ERROR_DUPLICATE_USERNAME                        0x803F // 32831
#define LSA_ERROR_DUPLICATE_GROUPNAME                       0x8040 // 32832
#define LSA_ERROR_DUPLICATE_CELLNAME                        0x8041 // 32833
#define LSA_ERROR_STRING_CONV_FAILED                        0x8042 // 32834
#define LSA_ERROR_INVALID_ACCOUNT                           0x8043 // 32835
#define LSA_ERROR_INVALID_PASSWORD                          0x8044 // 32836
#define LSA_ERROR_QUERY_CREATION_FAILED                     0x8045 // 32837
#define LSA_ERROR_NO_SUCH_OBJECT                            0x8046 // 32838
#define LSA_ERROR_DUPLICATE_USER_OR_GROUP                   0x8047 // 32839
#define LSA_ERROR_INVALID_KRB5_CACHE_TYPE                   0x8048 // 32840
#define LSA_ERROR_NOT_JOINED_TO_AD                          0x8049 // 32841
#define LSA_ERROR_FAILED_TO_SET_TIME                        0x804A // 32842
#define LSA_ERROR_NO_NETBIOS_NAME                           0x804B // 32843
#define LSA_ERROR_INVALID_NETLOGON_RESPONSE                 0x804C // 32844
#define LSA_ERROR_INVALID_OBJECTGUID                        0x804D // 32845
#define LSA_ERROR_INVALID_DOMAIN                            0x804E // 32846
#define LSA_ERROR_NO_DEFAULT_REALM                          0x804F // 32847
#define LSA_ERROR_NOT_SUPPORTED                             0x8050 // 32848
#define LSA_ERROR_LOGON_FAILURE                             0x8051 // 32849
#define LSA_ERROR_NO_SITE_INFORMATION                       0x8052 // 32850
#define LSA_ERROR_INVALID_LDAP_STRING                       0x8053 // 32851
#define LSA_ERROR_INVALID_LDAP_ATTR_VALUE                   0x8054 // 32852
#define LSA_ERROR_NULL_BUFFER                               0x8055 // 32853
#define LSA_ERROR_CLOCK_SKEW                                0x8056 // 32854
#define LSA_ERROR_KRB5_NO_KEYS_FOUND                        0x8057 // 32855
#define LSA_ERROR_SERVICE_NOT_AVAILABLE                     0x8058 // 32856
#define LSA_ERROR_INVALID_SERVICE_RESPONSE                  0x8059 // 32857
#define LSA_ERROR_NSS_ERROR                                 0x805A // 32858
#define LSA_ERROR_AUTH_ERROR                                0x805B // 32859
#define LSA_ERROR_INVALID_LDAP_DN                           0x805C // 32860
#define LSA_ERROR_NOT_MAPPED                                0x805D // 32861
#define LSA_ERROR_RPC_NETLOGON_FAILED                       0x805E // 32862
#define LSA_ERROR_ENUM_DOMAIN_TRUSTS_FAILED                 0x805F // 32863
#define LSA_ERROR_RPC_LSABINDING_FAILED                     0x8060 // 32864
#define LSA_ERROR_RPC_OPENPOLICY_FAILED                     0x8061 // 32865
#define LSA_ERROR_RPC_LSA_LOOKUPNAME2_FAILED                0x8062 // 32866
#define LSA_ERROR_RPC_SET_SESS_CREDS_FAILED                 0x8063 // 32867
#define LSA_ERROR_RPC_REL_SESS_CREDS_FAILED                 0x8064 // 32868
#define LSA_ERROR_RPC_CLOSEPOLICY_FAILED                    0x8065 // 32869
#define LSA_ERROR_RPC_LSA_LOOKUPNAME2_NOT_FOUND             0x8066 // 32870
#define LSA_ERROR_RPC_LSA_LOOKUPNAME2_FOUND_DUPLICATES      0x8067 // 32871
#define LSA_ERROR_NO_TRUSTED_DOMAIN_FOUND                   0x8068 // 32872
#define LSA_ERROR_INCOMPATIBLE_MODES_BETWEEN_TRUSTEDDOMAINS 0x8069 // 32873
#define LSA_ERROR_DCE_CALL_FAILED                           0x806A // 32874
#define LSA_ERROR_FAILED_TO_LOOKUP_DC                       0x806B // 32875
#define LSA_ERROR_INVALID_NSS_ARTEFACT_INFO_LEVEL           0x806C // 32876
#define LSA_ERROR_UNSUPPORTED_NSS_ARTEFACT_LEVEL            0x806D // 32877
#define LSA_ERROR_INVALID_USER_NAME                         0x806E // 32878
#define LSA_ERROR_INVALID_LOG_LEVEL                         0x806F // 32879
#define LSA_ERROR_INVALID_METRIC_TYPE                       0x8070 // 32880
#define LSA_ERROR_INVALID_METRIC_PACK                       0x8071 // 32881
#define LSA_ERROR_INVALID_METRIC_INFO_LEVEL                 0x8072 // 32882
#define LSA_ERROR_FAILED_STARTUP_PREREQUISITE_CHECK         0x8073 // 32883
#define LSA_ERROR_MAC_FLUSH_DS_CACHE_FAILED                 0x8074 // 32884
#define LSA_ERROR_LSA_SERVER_UNREACHABLE                    0x8075 // 32885
#define LSA_ERROR_INVALID_NSS_ARTEFACT_TYPE                 0x8076 // 32886
#define LSA_ERROR_INVALID_AGENT_VERSION                     0x8077 // 32887
#define LSA_ERROR_DOMAIN_IS_OFFLINE                         0x8078 // 32888
#define LSA_ERROR_INVALID_HOMEDIR_TEMPLATE                  0x8079 // 32889
#define LSA_ERROR_RPC_PARSE_SID_STRING                      0x807A // 32890
#define LSA_ERROR_RPC_LSA_LOOKUPSIDS_FAILED                 0x807B // 32891
#define LSA_ERROR_RPC_LSA_LOOKUPSIDS_NOT_FOUND              0x807C // 32892
#define LSA_ERORR_RPC_LSA_LOOKUPSIDS_FOUND_DUPLICATES       0x807D // 32893
#define LSA_ERROR_PASSWORD_RESTRICTION                      0x807E // 32894
#define LSA_ERROR_OBJECT_NOT_ENABLED                        0x807F // 32895
#define LSA_ERROR_NO_MORE_NSS_ARTEFACTS                     0x8080 // 32896
#define LSA_ERROR_INVALID_NSS_MAP_NAME                      0x8081 // 32897
#define LSA_ERROR_INVALID_NSS_KEY_NAME                      0x8082 // 32898
#define LSA_ERROR_NO_SUCH_NSS_KEY                           0x8083 // 32899
#define LSA_ERROR_NO_SUCH_NSS_MAP                           0x8084 // 32900
#define LSA_ERROR_RPC_ERROR                                 0x8085 // 32901
#define LSA_ERROR_LDAP_SERVER_UNAVAILABLE                   0x8086 // 32902
#define LSA_ERROR_CREATE_KEY_FAILED                         0x8087 // 32903
#define LSA_ERROR_CANNOT_DETECT_USER_PROCESSES              0x8088 // 32904
#define LSA_ERROR_TRACE_NOT_INITIALIZED                     0x8089 // 32905
#define LSA_ERROR_NO_SUCH_TRACE_FLAG                        0x808A // 32906
#define LSA_ERROR_DCERPC_ERROR                              0x808B // 32907
#define LSA_ERROR_INVALID_RPC_SERVER                        0x808C // 32908
#define LSA_ERROR_RPC_SERVER_REGISTRATION_ERROR             0x808D // 32909
#define LSA_ERROR_RPC_SERVER_RUNTIME_ERROR                  0x808E // 32910
#define LSA_ERROR_DOMAIN_IN_USE                             0x808F // 32911
#define LSA_ERROR_SAM_DATABASE_ERROR                        0x8090 // 32912
#define LSA_ERROR_SAM_INIT_ERROR                            0x8091 // 32913
#define LSA_ERROR_OBJECT_IN_USE                             0x8092 // 32914
#define LSA_ERROR_NO_SUCH_ATTRIBUTE                         0x8093 // 32915
#define LSA_ERROR_GET_DC_NAME_FAILED                        0x8094 // 32916
#define LSA_ERROR_INVALID_ATTRIBUTE_VALUE                   0x8095 // 32917
#define LSA_ERROR_NO_ATTRIBUTE_VALUE                        0x8096 // 32918
#define LSA_ERROR_MEMBER_IN_LOCAL_GROUP                     0x8098 // 32919
#define LSA_ERROR_MEMBER_NOT_IN_LOCAL_GROUP                 0x8099 // 32920
#define LSA_ERROR_KRB5_S_PRINCIPAL_UNKNOWN                  0x809A // 32921
#define LSA_ERROR_INVALID_GROUP                             0x809B // 32922
#define LSA_ERROR_SENTINEL                                  0x809C // 32923

/* range 0x8600 - 0x8650 are reserved for GSS specific errors */

#define LSA_ERROR_BAD_MECH                                  0x8600  // 34304
#define LSA_ERROR_BAD_NAMETYPE                              0x8601  // 34305
#define LSA_ERROR_BAD_NAME                                  0x8602  // 34306
#define LSA_ERROR_INVALID_CONTEXT                           0x8603  // 34307
#define LSA_ERROR_INVALID_CREDENTIAL                        0x8604  // 34308
#define LSA_ERROR_NO_CONTEXT                                0x8605  // 34309
#define LSA_ERROR_NO_CRED                                   0x8606  // 34310
#define LSA_ERROR_INVALID_TOKEN                             0x8607  // 34311
#define LSA_ERROR_UNSUPPORTED_SUBPROTO                      0x8608  // 34312
#define LSA_ERROR_UNSUPPORTED_CRYPTO_OP                     0x8609  // 34313

#define LSA_ERROR_MASK(_e_)                                 (_e_ & 0x8000)

/* WARNINGS */
#define LSA_WARNING_CONTINUE_NEEDED                         0x7001

#define LWPS_ERROR_INVALID_ACCOUNT                          0x4016 // 16406

#endif /* LSA_ERRORS_DEFINED */

typedef DWORD LSA_DS_FLAGS, *PLSA_DS_FLAGS;

#define LSA_DS_DNS_CONTROLLER_FLAG  0x20000000
#define LSA_DS_DNS_DOMAIN_FLAG      0x40000000
#define LSA_DS_DNS_FOREST_FLAG      0x80000000
#define LSA_DS_DS_FLAG              0x00000010
#define LSA_DS_GC_FLAG              0x00000004
#define LSA_DS_KDC_FLAG             0x00000020
#define LSA_DS_PDC_FLAG             0x00000001
#define LSA_DS_TIMESERV_FLAG        0x00000040
#define LSA_DS_WRITABLE_FLAG        0x00000100

typedef DWORD LSA_DM_DOMAIN_FLAGS, *PLSA_DM_DOMAIN_FLAGS;

#define LSA_DM_DOMAIN_FLAG_PRIMARY               0x00000001
#define LSA_DM_DOMAIN_FLAG_OFFLINE               0x00000002
#define LSA_DM_DOMAIN_FLAG_FORCE_OFFLINE         0x00000004
#define LSA_DM_DOMAIN_FLAG_TRANSITIVE_1WAY_CHILD 0x00000008
#define LSA_DM_DOMAIN_FLAG_FOREST_ROOT           0x00000010
#define LSA_DM_DOMAIN_FLAG_GC_OFFLINE            0x00000020


#define LSA_DM_DOMAIN_FLAGS_VALID_MASK \
    ( \
        LSA_DM_DOMAIN_FLAG_PRIMARY | \
        LSA_DM_DOMAIN_FLAG_OFFLINE | \
        LSA_DM_DOMAIN_FLAG_FORCE_OFFLINE | \
        LSA_DM_DOMAIN_FLAG_TRANSITIVE_1WAY_CHILD | \
        LSA_DM_DOMAIN_FLAG_FOREST_ROOT | \
        LSA_DM_DOMAIN_FLAG_GC_OFFLINE | \
        0 \
    )

typedef DWORD LSA_DM_STATE_FLAGS, *PLSA_DM_STATE_FLAGS;

/// Controls whether to enable offline reporting.
/// Offline state is always tracked internally,
/// but this controls whether to honor that state.
#define LSA_DM_STATE_FLAG_OFFLINE_ENABLED        0x00000001
/// Whether forced globally offline (by user).
#define LSA_DM_STATE_FLAG_FORCE_OFFLINE          0x00000002
/// Whether globally offline due to media sense.
#define LSA_DM_STATE_FLAG_MEDIA_SENSE_OFFLINE    0x00000004

#define LSA_DM_STATE_FLAGS_VALID_MASK \
    ( \
        LSA_DM_STATE_FLAG_OFFLINE_ENABLED | \
        LSA_DM_STATE_FLAG_FORCE_OFFLINE | \
        LSA_DM_STATE_FLAG_MEDIA_SENSE_OFFLINE | \
        0 \
    )

typedef DWORD LSA_TRUST_TYPE, *PLSA_TRUST_TYPE;

#define LSA_TRUST_TYPE_DOWNLEVEL            0x00000001
#define LSA_TRUST_TYPE_UPLEVEL              0x00000002
#define LSA_TRUST_TYPE_MIT                  0x00000003
#define LSA_TRUST_TYPE_DCE                  0x00000004

typedef DWORD LSA_TRUST_ATTRIBUTE, *PLSA_TRUST_ATTRIBUTE;

#define LSA_TRUST_ATTRIBUTE_NON_TRANSITIVE     0x00000001
#define LSA_TRUST_ATTRIBUTE_UPLEVEL_ONLY       0x00000002
#define LSA_TRUST_ATTRIBUTE_FILTER_SIDS        0x00000004
#define LSA_TRUST_ATTRIBUTE_FOREST_TRANSITIVE  0x00000008
#define LSA_TRUST_ATTRIBUTE_CROSS_ORGANIZATION 0x00000010
#define LSA_TRUST_ATTRIBUTE_WITHIN_FOREST      0x00000020

typedef DWORD LSA_TRUST_FLAG, *PLSA_TRUST_FLAG;

#define LSA_TRUST_FLAG_IN_FOREST    0x00000001
#define LSA_TRUST_FLAG_OUTBOUND     0x00000002
#define LSA_TRUST_FLAG_TREEROOT     0x00000004
#define LSA_TRUST_FLAG_PRIMARY      0x00000008
#define LSA_TRUST_FLAG_NATIVE       0x00000010
#define LSA_TRUST_FLAG_INBOUND      0x00000020

typedef DWORD LSA_TRUST_DIRECTION;

#define LSA_TRUST_DIRECTION_UNKNOWN  0x00000000
#define LSA_TRUST_DIRECTION_ZERO_WAY 0x00000001
#define LSA_TRUST_DIRECTION_ONE_WAY  0x00000002
#define LSA_TRUST_DIRECTION_TWO_WAY  0x00000003
#define LSA_TRUST_DIRECTION_SELF     0x00000004

typedef DWORD LSA_TRUST_MODE;

#define LSA_TRUST_MODE_UNKNOWN       0x00000000
#define LSA_TRUST_MODE_EXTERNAL      0x00000001
#define LSA_TRUST_MODE_MY_FOREST     0x00000002
#define LSA_TRUST_MODE_OTHER_FOREST  0x00000003

#define LSA_NIS_MAP_NAME_NETGROUPS  "netgroup"
#define LSA_NIS_MAP_NAME_SERVICES   "services"
#define LSA_NIS_MAP_NAME_AUTOMOUNTS "automounts"

typedef DWORD LSA_NIS_MAP_QUERY_FLAGS;

#define LSA_NIS_MAP_QUERY_KEYS       0x00000001
#define LSA_NIS_MAP_QUERY_VALUES     0x00000002
#define LSA_NIS_MAP_QUERY_ALL        (LSA_NIS_MAP_QUERY_KEYS | LSA_NIS_MAP_QUERY_VALUES)

typedef DWORD LSA_FIND_FLAGS, *PLSA_FIND_FLAGS;

#define LSA_FIND_FLAGS_NSS 0x00000001

/*
 * Tracing support
 */
#define LSA_TRACE_FLAG_USER_GROUP_QUERIES        1
#define LSA_TRACE_FLAG_AUTHENTICATION            2
#define LSA_TRACE_FLAG_USER_GROUP_ADMINISTRATION 3
#define LSA_TRACE_FLAG_SENTINEL                  4

typedef struct __LSA_TRACE_INFO
{
    DWORD   dwTraceFlag;
    BOOLEAN bStatus;
} LSA_TRACE_INFO, *PLSA_TRACE_INFO;

typedef struct __LSA_TRACE_INFO_LIST
{
    DWORD dwNumFlags;
    PLSA_TRACE_INFO pTraceInfoArray;
} LSA_TRACE_INFO_LIST, *PLSA_TRACE_INFO_LIST;


/*
 * Logging
 */
typedef enum
{
	LSA_LOG_LEVEL_ALWAYS = 0,
	LSA_LOG_LEVEL_ERROR,
    LSA_LOG_LEVEL_WARNING,
    LSA_LOG_LEVEL_INFO,
    LSA_LOG_LEVEL_VERBOSE,
    LSA_LOG_LEVEL_DEBUG,
    LSA_LOG_LEVEL_TRACE
} LsaLogLevel;

typedef enum
{
    LSA_LOG_TARGET_DISABLED = 0,
    LSA_LOG_TARGET_CONSOLE,
    LSA_LOG_TARGET_FILE,
    LSA_LOG_TARGET_SYSLOG
} LsaLogTarget;

typedef VOID (*PFN_LSA_LOG_MESSAGE)(
		            HANDLE      hLog,
					LsaLogLevel logLevel,
					PCSTR       pszFormat,
					va_list     msgList
					);

typedef struct __LSA_LOG_INFO {
    LsaLogLevel  maxAllowedLogLevel;
    LsaLogTarget logTarget;
    PSTR         pszPath;
} LSA_LOG_INFO, *PLSA_LOG_INFO;

/**
 * @ingroup user
 * @brief User info structure -- level 0
 *
 * Describes the basic attributes of a user,
 * particularly those which are present in the
 * classic UNIX passwd structure.
 */
typedef struct __LSA_USER_INFO_0
{
    /** @brief User ID */
    uid_t uid;
    /** @brief Primary group ID */
    gid_t gid;
    /** @brief Username (alias) */
    PSTR  pszName;
    /** @brief Password (may be NULL) */
    PSTR  pszPasswd;
    /** @brief Comment */
    PSTR  pszGecos;
    /** @brief Login shell path */
    PSTR  pszShell;
    /** @brief Home directory path */
    PSTR  pszHomedir;
    /** @brief Windows SID in string form (may be NULL) */
    PSTR  pszSid;
} LSA_USER_INFO_0, *PLSA_USER_INFO_0;

/**
 * @ingroup user
 * @brief User info structure -- level 1
 *
 * Describes everything about a user included in #__LSA_USER_INFO_0
 * in addition to several attributes which tend to be applicable
 * only in Windows network environments.
 */
typedef struct __LSA_USER_INFO_1
{
#ifndef DOXYGEN
    union
    {
        struct
        {
#endif
            uid_t uid;
            gid_t gid;
            PSTR  pszName;
            PSTR  pszPasswd;
            PSTR  pszGecos;
            PSTR  pszShell;
            PSTR  pszHomedir;
            PSTR  pszSid;
#ifndef DOXYGEN
        };
        LSA_USER_INFO_0 info0;
    };
#endif
    /** @brief User object DN */
    PSTR  pszDN;
    /** @brief User's Kerberos UPN */
    PSTR  pszUPN;
    /** @brief Whether the UPN is explicit or implicit */
    DWORD bIsGeneratedUPN;
    /** @brief Whether the user is from a local account database */
    DWORD bIsLocalUser;
    /** @brief LM hash of the user's password */
    PBYTE pLMHash;
    /** @brief Length of the LM hash */
    DWORD dwLMHashLen;
    /** @brief NT hash of the user's password */
    PBYTE pNTHash;
    /** @brief Length of the NT hash */
    DWORD dwNTHashLen;
} LSA_USER_INFO_1, *PLSA_USER_INFO_1;

/**
 * @ingroup user
 * @brief User info structure -- level 2
 *
 * Describes everything about a user included in #__LSA_USER_INFO_1
 * in addition to attributes which describe the password expiry
 * and account status of the user.
 */
typedef struct __LSA_USER_INFO_2
{
#ifndef DOXYGEN
    union
    {
        struct
        {
#endif
            uid_t uid;
            gid_t gid;
            PSTR  pszName;
            PSTR  pszPasswd;
            PSTR  pszGecos;
            PSTR  pszShell;
            PSTR  pszHomedir;
            PSTR  pszSid;
            PSTR  pszDN;
            PSTR  pszUPN;
            DWORD bIsGeneratedUPN;
            DWORD bIsLocalUser;
            PBYTE pLMHash;
            DWORD dwLMHashLen;
            PBYTE pNTHash;
            DWORD dwNTHashLen;
#ifndef DOXYGEN
        };
        LSA_USER_INFO_1 info1;
    };
#endif
    /** @brief Number of days until the user's password will expire */
    DWORD   dwDaysToPasswordExpiry;
    /** @brief Whether the user's password has expired */
    BOOLEAN bPasswordExpired;
    /** @brief Whether the user's password will never expire */
    BOOLEAN bPasswordNeverExpires;
    /** @brief Whether the user should be prompted to change password */
    BOOLEAN bPromptPasswordChange;
    /** @brief Whether the user can change password */
    BOOLEAN bUserCanChangePassword;
    /** @brief Whether the account is disabled */
    BOOLEAN bAccountDisabled;
    /** @brief Whether the account is expired */
    BOOLEAN bAccountExpired;
    /** @brief Whether the account is locked */
    BOOLEAN bAccountLocked;
} LSA_USER_INFO_2, *PLSA_USER_INFO_2;

typedef struct __LSA_USER_INFO_LIST
{
    DWORD dwUserInfoLevel;
    DWORD dwNumUsers;
    union _USER_INFO_LIST
    {
        PLSA_USER_INFO_0* ppInfoList0;
        PLSA_USER_INFO_1* ppInfoList1;
        PLSA_USER_INFO_2* ppInfoList2;
    }ppUserInfoList;
} LSA_USER_INFO_LIST, *PLSA_USER_INFO_LIST;

typedef struct __LSA_USER_MOD_INFO
{
    uid_t uid;

    struct _usermod_actions {
        BOOLEAN bEnableUser;
        BOOLEAN bDisableUser;
        BOOLEAN bUnlockUser;
        BOOLEAN bSetChangePasswordOnNextLogon;
        BOOLEAN bSetPasswordNeverExpires;
        BOOLEAN bSetPasswordMustExpire;
        BOOLEAN bAddToGroups;
        BOOLEAN bRemoveFromGroups;
        BOOLEAN bSetAccountExpiryDate;
    } actions;

    PSTR    pszAddToGroups;
    PSTR    pszRemoveFromGroups;
    PSTR    pszExpiryDate;

} LSA_USER_MOD_INFO, *PLSA_USER_MOD_INFO;

typedef struct __LSA_GROUP_INFO_0
{
    gid_t gid;
    PSTR  pszName;
    PSTR  pszSid;
} LSA_GROUP_INFO_0, *PLSA_GROUP_INFO_0;

typedef struct __LSA_GROUP_INFO_1
{
    union
    {
        struct
        {
            gid_t gid;
            PSTR  pszName;
            PSTR  pszSid;
        };
        LSA_GROUP_INFO_0 info0;
    };
    PSTR  pszDN;
    PSTR  pszPasswd;
    PSTR* ppszMembers;
} LSA_GROUP_INFO_1, *PLSA_GROUP_INFO_1;

typedef struct __LSA_GROUP_INFO_LIST
{
    DWORD dwGroupInfoLevel;
    DWORD dwNumGroups;
    union _GROUP_INFO_LIST
    {
        PLSA_GROUP_INFO_0* ppInfoList0;
        PLSA_GROUP_INFO_1* ppInfoList1;
    }ppGroupInfoList;
} LSA_GROUP_INFO_LIST, *PLSA_GROUP_INFO_LIST;

typedef struct __LSA_GROUP_MEMBER_INFO
{
    PSTR pszDN;
    PSTR pszSid;
} LSA_GROUP_MEMBER_INFO, *PLSA_GROUP_MEMBER_INFO;

typedef struct __LSA_GROUP_MOD_INFO
{
    gid_t gid;

    struct _groupmod_actions {
        BOOLEAN bAddMembers;
        BOOLEAN bRemoveMembers;
    } actions;

    DWORD dwAddMembersNum;
    PLSA_GROUP_MEMBER_INFO pAddMembers;

    DWORD dwRemoveMembersNum;
    PLSA_GROUP_MEMBER_INFO pRemoveMembers;

} LSA_GROUP_MOD_INFO, *PLSA_GROUP_MOD_INFO;

typedef struct __LSA_ENUM_OBJECTS_INFO
{
    DWORD dwObjectInfoLevel;
    DWORD dwNumMaxObjects;
    PSTR  pszGUID;
} LSA_ENUM_OBJECTS_INFO, *PLSA_ENUM_OBJECTS_INFO;

typedef struct __LSA_NSS_ARTEFACT_INFO_0
{
    PSTR  pszName;
    PSTR  pszValue;
} LSA_NSS_ARTEFACT_INFO_0, *PLSA_NSS_ARTEFACT_INFO_0;

typedef struct __LSA_NSS_ARTEFACT_INFO_LIST
{
    DWORD dwNssArtefactInfoLevel;
    DWORD dwNumNssArtefacts;
    union _NSS_ARTEFACT_INFO_LIST
    {
        PLSA_NSS_ARTEFACT_INFO_0* ppInfoList0;
    }ppNssArtefactInfoList;
} LSA_NSS_ARTEFACT_INFO_LIST, *PLSA_NSS_ARTEFACT_INFO_LIST;


typedef struct _SEC_BUFFER {
    USHORT length;
    USHORT maxLength;
    PBYTE  buffer;
} SEC_BUFFER, *PSEC_BUFFER;

/* static buffer secbufer */
#define S_BUFLEN 24

typedef struct _SEC_BUFFER_S {
    USHORT length;
    USHORT maxLength;
    BYTE buffer[S_BUFLEN];
} SEC_BUFFER_S, *PSEC_BUFFER_S;

typedef UINT8 ADAccountType;

#define AccountType_NotFound 0
#define AccountType_Group 1
#define AccountType_User 2
#define AccountType_Domain 3

typedef struct __LSA_SID_INFO
{
    ADAccountType accountType;
    PSTR          pszSamAccountName;
    PSTR          pszDomainName;
} LSA_SID_INFO, *PLSA_SID_INFO;

typedef struct __LSA_FIND_NAMES_BY_SIDS
{
    size_t sCount;
    PLSA_SID_INFO pSIDInfoList;
    CHAR chDomainSeparator;
} LSA_FIND_NAMES_BY_SIDS, *PLSA_FIND_NAMES_BY_SIDS;

typedef struct __LSA_METRIC_PACK_0
{
    UINT64 failedAuthentications;
    UINT64 failedUserLookupsByName;
    UINT64 failedUserLookupsById;
    UINT64 failedGroupLookupsByName;
    UINT64 failedGroupLookupsById;
    UINT64 failedOpenSession;
    UINT64 failedCloseSession;
    UINT64 failedChangePassword;
    UINT64 unauthorizedAccesses;

} LSA_METRIC_PACK_0, *PLSA_METRIC_PACK_0;

typedef struct __LSA_METRIC_PACK_1
{
    UINT64 successfulAuthentications;
    UINT64 failedAuthentications;
    UINT64 rootUserAuthentications;
    UINT64 successfulUserLookupsByName;
    UINT64 failedUserLookupsByName;
    UINT64 successfulUserLookupsById;
    UINT64 failedUserLookupsById;
    UINT64 successfulGroupLookupsByName;
    UINT64 failedGroupLookupsByName;
    UINT64 successfulGroupLookupsById;
    UINT64 failedGroupLookupsById;
    UINT64 successfulOpenSession;
    UINT64 failedOpenSession;
    UINT64 successfulCloseSession;
    UINT64 failedCloseSession;
    UINT64 successfulChangePassword;
    UINT64 failedChangePassword;
    UINT64 unauthorizedAccesses;

} LSA_METRIC_PACK_1, *PLSA_METRIC_PACK_1;

typedef struct __LSA_METRIC_PACK
{
    DWORD dwInfoLevel;
    union _METRIC_PACK
    {
        PLSA_METRIC_PACK_0 pMetricPack0;
        PLSA_METRIC_PACK_1 pMetricPack1;
    }pMetricPack;
} LSA_METRIC_PACK, *PLSA_METRIC_PACK;

typedef enum
{
    LSA_PROVIDER_MODE_UNKNOWN = 0,
    LSA_PROVIDER_MODE_UNPROVISIONED,
    LSA_PROVIDER_MODE_DEFAULT_CELL,
    LSA_PROVIDER_MODE_NON_DEFAULT_CELL,
    LSA_PROVIDER_MODE_LOCAL_SYSTEM
} LsaAuthProviderMode;

typedef enum
{
    LSA_AUTH_PROVIDER_SUBMODE_UNKNOWN = 0,
    LSA_AUTH_PROVIDER_SUBMODE_SCHEMA,
    LSA_AUTH_PROVIDER_SUBMODE_NONSCHEMA
} LsaAuthProviderSubMode;

typedef enum
{
    LSA_AUTH_PROVIDER_STATUS_UNKNOWN = 0,
    LSA_AUTH_PROVIDER_STATUS_ONLINE,
    LSA_AUTH_PROVIDER_STATUS_OFFLINE,
    LSA_AUTH_PROVIDER_STATUS_FORCED_OFFLINE
} LsaAuthProviderStatus;

typedef struct __LSA_DC_INFO
{
    PSTR         pszName;
    PSTR         pszAddress;
    PSTR         pszSiteName;
    LSA_DS_FLAGS dwFlags;
} LSA_DC_INFO, *PLSA_DC_INFO;

typedef struct __LSA_TRUSTED_DOMAIN_INFO
{
    PSTR                pszDnsDomain;
    PSTR                pszNetbiosDomain;
    PSTR                pszTrusteeDnsDomain;
    PSTR                pszDomainSID;
    PSTR                pszDomainGUID;
    PSTR                pszForestName;
    PSTR                pszClientSiteName;
    LSA_TRUST_FLAG      dwTrustFlags;
    LSA_TRUST_TYPE      dwTrustType;
    LSA_TRUST_ATTRIBUTE dwTrustAttributes;
    LSA_TRUST_DIRECTION dwTrustDirection;
    LSA_TRUST_MODE      dwTrustMode;
    LSA_DM_DOMAIN_FLAGS dwDomainFlags;
    PLSA_DC_INFO        pDCInfo;
    PLSA_DC_INFO        pGCInfo;
} LSA_TRUSTED_DOMAIN_INFO, *PLSA_TRUSTED_DOMAIN_INFO;

typedef struct __LSA_AUTH_PROVIDER_STATUS
{
    PSTR                     pszId;
    LsaAuthProviderMode      mode;
    LsaAuthProviderSubMode   subMode;
    LsaAuthProviderStatus    status;
    PSTR                     pszDomain;
    PSTR                     pszForest;
    PSTR                     pszSite;
    PSTR                     pszCell;
    DWORD                    dwNetworkCheckInterval;
    DWORD                    dwNumTrustedDomains;
    PLSA_TRUSTED_DOMAIN_INFO pTrustedDomainInfoArray;
} LSA_AUTH_PROVIDER_STATUS, *PLSA_AUTH_PROVIDER_STATUS;

typedef struct __LSA_VERSION
{
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwBuild;
} LSA_VERSION, *PLSA_VERSION;

typedef struct __LSASTATUS
{
    DWORD dwUptime;

    LSA_VERSION version;

    DWORD dwCount;
    PLSA_AUTH_PROVIDER_STATUS pAuthProviderStatusList;

} LSASTATUS, *PLSASTATUS;


/*
 * AuthenticateUserEx() parameters
 */

typedef enum
{
	LSA_MARSHALL_DATA = 1,
	LSA_UNMARSHALL_DATA
} LsaMarshallType;

typedef enum
{
	LSA_AUTH_PLAINTEXT = 1,
	LSA_AUTH_CHAP
} LsaAuthType;

typedef struct __LSA_AUTH_CLEARTEXT_PARAM
{
	PSTR pszPassword;

} LSA_AUTH_CLEARTEXT_PARAM, *PLSA_AUTH_CLEARTEXT_PARAM;

typedef struct __LSA_DATA_BLOB
{
	DWORD dwLen;
	PBYTE pData;

} LSA_DATA_BLOB, *PLSA_DATA_BLOB;

typedef struct __LSA_AUTH_CHAP_PARAM
{
	PLSA_DATA_BLOB pChallenge;
	PLSA_DATA_BLOB pLM_resp;
	PLSA_DATA_BLOB pNT_resp;

} LSA_AUTH_CHAP_PARAM, *PLSA_AUTH_CHAP_PARAM;

typedef struct __LSA_AUTH_USER_PARAMS
{
	LsaAuthType AuthType;
	PSTR pszAccountName;
	PSTR pszDomain;
	PSTR pszWorkstation;
	union _PASS{
		LSA_AUTH_CLEARTEXT_PARAM clear;
		LSA_AUTH_CHAP_PARAM      chap;
	} pass;

} LSA_AUTH_USER_PARAMS, *PLSA_AUTH_USER_PARAMS;

#define LSA_MAX_SID_SUB_AUTHORITIES  15

typedef struct __LSA_SID_ATTRIB
{
	PSTR     pszSid;
	DWORD    dwAttrib;

} LSA_SID_ATTRIB, *PLSA_SID_ATTRIB;

typedef struct __LSA_RID_ATTRIB
{
    UINT32   Rid;
    DWORD    dwAttrib;

} LSA_RID_ATTRIB, *PLSA_RID_ATTRIB;

#define LSA_SID_ATTR_GROUP_MANDATORY		0x00000001
#define LSA_SID_ATTR_GROUP_ENABLED_BY_DEFAULT	0x00000002
#define LSA_SID_ATTR_GROUP_ENABLED 		0x00000004
#define LSA_SID_ATTR_GROUP_OWNER 		0x00000008
#define LSA_SID_ATTR_GROUP_USEFOR_DENY_ONLY 	0x00000010
#define LSA_SID_ATTR_GROUP_RESOURCE 		0x20000000
#define LSA_SID_ATTR_GROUP_LOGON_ID 		0xC0000000

typedef struct __LSA_AUTH_USER_INFO
{
	DWORD dwUserFlags;

	PSTR pszAccount;
	PSTR pszUserPrincipalName;
	PSTR pszFullName;
	PSTR pszDomain;
	PSTR pszDnsDomain;

	DWORD dwAcctFlags;
	PLSA_DATA_BLOB  pSessionKey;
	PLSA_DATA_BLOB  pLmSessionKey;

	UINT16 LogonCount;
	UINT16 BadPasswordCount;

	INT64 LogonTime;
	INT64 LogoffTime;
	INT64 KickoffTime;
	INT64 LastPasswordChange;
	INT64 CanChangePassword;
	INT64 MustChangePassword;

	PSTR pszLogonServer;
	PSTR pszLogonScript;
	PSTR pszProfilePath;
	PSTR pszHomeDirectory;
	PSTR pszHomeDrive;

    PSTR pszDomainSid;
	DWORD dwUserRid;
	DWORD dwPrimaryGroupRid;

	DWORD dwNumRids;
	PLSA_RID_ATTRIB pRidAttribList;

	DWORD dwNumSids;
	PLSA_SID_ATTRIB pSidAttribList;

} LSA_AUTH_USER_INFO, *PLSA_AUTH_USER_INFO;

/**
 * @ingroup connection
 * @brief Open connection to local LSASS server
 *
 * Creates a connection handle to the local LSASS server.
 *
 * @param[out] phConnection the created connection handle
 * @retval LSA_ERROR_SUCCESS success
 * @retval ECONNREFUSED the connection was refused
 * @retval ENOENT the local domain socket was not present
 */
DWORD
LsaOpenServer(
    PHANDLE phConnection
    );

DWORD
LsaBuildLogInfo(
    LsaLogLevel    maxAllowedLogLevel,
    LsaLogTarget   logTarget,
    PCSTR          pszPath,
    PLSA_LOG_INFO* ppLogInfo
    );

DWORD
LsaSetLogLevel(
    HANDLE      hLsaConnection,
    LsaLogLevel logLevel
    );

DWORD
LsaGetLogInfo(
    HANDLE         hLsaConnection,
    PLSA_LOG_INFO* ppLogInfo
    );

DWORD
LsaSetLogInfo(
    HANDLE        hLsaConnection,
    PLSA_LOG_INFO pLogInfo
    );

VOID
LsaFreeLogInfo(
    PLSA_LOG_INFO pLogInfo
    );

DWORD
LsaSetTraceFlags(
    HANDLE          hLsaConnection,
    PLSA_TRACE_INFO pTraceFlagArray,
    DWORD           dwNumFlags
    );

DWORD
LsaEnumTraceFlags(
    HANDLE           hLsaConnection,
    PLSA_TRACE_INFO* ppTraceFlagArray,
    PDWORD           pdwNumFlags
    );

DWORD
LsaGetTraceFlag(
    HANDLE           hLsaConnection,
    DWORD            dwTraceFlag,
    PLSA_TRACE_INFO* ppTraceFlag
    );

/**
 * @ingroup group
 * @brief Create new group
 *
 * Creates a new group in the local account database.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pGroupInfo a group info structure
 * @param[in] dwGroupInfoLevel the info level of the provided group info structure
 * @retval LSA_ERROR_SUCCESS success
 * @retval EPERM the owner of the current process is not authorized to create groups
 */
DWORD
LsaAddGroup(
    HANDLE hLsaConnection,
    PVOID  pGroupInfo,
    DWORD  dwGroupInfoLevel
    );

DWORD
LsaModifyGroup(
    HANDLE hLsaConnection,
    PLSA_GROUP_MOD_INFO pGroupModInfo
    );

/**
 * @ingroup group
 * @brief Delete a group by ID
 *
 * Deletes a group from the local account database based on its UNIX group ID
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] gid the group ID of the group to delete
 * @retval LSA_ERROR_SUCCESS success
 * @retval EPERM the owner of the current process is not authorized to delete groups
 * @retval LSA_ERROR_NO_SUCH_GROUP the specified group ID did not match any local group
 */
DWORD
LsaDeleteGroupById(
    HANDLE hLsaConnection,
    gid_t  gid
    );

/**
 * @ingroup group
 * @brief Delete a group by name
 *
 * Deletes a group from the local account database based on its name
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pszName the name of the group to delete
 * @retval LSA_ERROR_SUCCESS success
 * @retval EPERM the owner of the current process is not authorized to delete groups
 * @retval LSA_ERROR_NO_SUCH_GROUP the specified group name did not match any local group
 */
DWORD
LsaDeleteGroupByName(
    HANDLE hLsaConnection,
    PCSTR  pszName
    );

/**
 * @ingroup user
 * @brief Look up group IDs by username
 *
 * Looks up the group IDs for the groups which a user is a member of based on the user's login name.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pszUserName the login name of the user
 * @param[out] pdwGroupFound the number of groups find
 * @param[out] ppGidResults a heap-allocated list of group IDs
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_USER the specified user name did not match any known user
 */
DWORD
LsaGetGidsForUserByName(
    HANDLE  hLsaConnection,
    PCSTR   pszUserName,
    PDWORD  pdwGroupFound,
    gid_t** ppGidResults
    );

/**
 * @ingroup user
 * @brief Look up groups by user ID
 *
 * Looks up information on groups which a user is a member of based on user's login name.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pszUserName the login name of the user
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwGroupInfoLevel the desired info level for the returned group info structures
 * @param[out] pdwGroupsFound the number of groups find
 * @param[out] pppGroupInfoList a heap-allocated list of group info structures
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_USER the specified user ID did not match any known user
 */
LW_DWORD
LsaGetGroupsForUserByName(
    LW_IN LW_HANDLE hLsaConnection,
    LW_IN LW_PCSTR pszUserName,
    LW_IN LSA_FIND_FLAGS FindFlags,
    LW_IN LW_DWORD dwGroupInfoLevel,
    LW_OUT LW_PDWORD pdwGroupsFound,
    LW_OUT LW_PVOID** pppGroupInfoList
    );

/**
 * @ingroup user
 * @brief Look up groups by user ID
 *
 * Looks up information on groups which a user is a member of based on user ID.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] uid the user ID of the user
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwGroupInfoLevel the desired info level for the returned group info structures
 * @param[out] pdwGroupsFound the number of groups find
 * @param[out] pppGroupInfoList a heap-allocated list of group info structures
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_USER the specified user ID did not match any known user
 */
DWORD
LsaGetGroupsForUserById(
    HANDLE  hLsaConnection,
    uid_t   uid,
    LSA_FIND_FLAGS FindFlags,
    DWORD   dwGroupInfoLevel,
    PDWORD  pdwGroupsFound,
    PVOID** pppGroupInfoList
    );

/**
 * @ingroup group
 * @brief Look up group by name
 *
 * Looks up information on a group by its name.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pszGroupName the name of the group
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwGroupInfoLevel the desired info level for the returned group info structure
 * @param[out] ppGroupInfo a heap-allocated group info structure for the found group
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_GROUP the specified name did not match any known group
 */
DWORD
LsaFindGroupByName(
    HANDLE hLsaConnection,
    PCSTR  pszGroupName,
    LSA_FIND_FLAGS FindFlags,
    DWORD  dwGroupInfoLevel,
    PVOID* ppGroupInfo
    );

/**
 * @ingroup group
 * @brief Look up group by ID
 *
 * Looks up information on a group by its group ID.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] gid the group ID of the group
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwGroupInfoLevel the desired info level for the returned group info structure
 * @param[out] ppGroupInfo a heap-allocated group info structure for the found group
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_GROUP the specified group ID did not match any known group
 */
DWORD
LsaFindGroupById(
    HANDLE hLsaConnection,
    gid_t  gid,
    LSA_FIND_FLAGS FindFlags,
    DWORD  dwGroupInfoLevel,
    PVOID* ppGroupInfo
    );

/**
 * @ingroup group
 * @brief Begin group enumeration
 *
 * Begins an enumeration of all known groups.  This function returns an
 * enumeration handle which can be used with #LsaEnumGroups() to fetch
 * lists of groups in increments of up to dwMaxNumGroups.
 *
 * You must call #LsaEndEnumGroups() on the enumeration handle when
 * finished with the enumeration.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] dwGroupInfoLevel the desired info level for the group info structures
 * returned during the enumeration
 * @param[in] dwMaxNumGroups the maximum number of group info structures to
 * return in each subsequent call to #LsaEnumGroups()
 * @param[in] FindFlags options for the lookup operation
 * @param[out] phResume the created enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaBeginEnumGroups(
    HANDLE  hLsaConnection,
    DWORD   dwGroupInfoLevel,
    DWORD   dwMaxNumGroups,
    LSA_FIND_FLAGS FindFlags,
    PHANDLE phResume
    );

/**
 * @ingroup group
 * @brief Begin group enumeration with online check option
 *
 * Begins an enumeration of all known groups.  This function returns an
 * enumeration handle which can be used with #LsaEnumGroups() to fetch
 * lists of groups in increments of up to dwMaxNumGroups.  Compared to
 * #LsaBeginEnumGroups(), it supports an extra option that allows the
 * query to be restricted to groups that are local or cached, avoiding
 * excess traffic when large numbers of groups are present in a networked
 * identity database (e.g. AD).
 *
 * You must call #LsaEndEnumGroups() on the enumeration handle when
 * finished with the enumeration.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] dwGroupInfoLevel the desired info level for the group info structures
 * returned during the enumeration
 * @param[in] dwMaxNumGroups the maximum number of group info structures to
 * return in each subsequent call to #LsaEnumGroups()
 * @param[in] bCheckGroupMembersOnline TRUE if networked databases should be
 * consulted, FALSE if only local databases or caches should be searched
 * @param[in] FindFlags options for the lookup operation
 * @param[out] phResume the created enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaBeginEnumGroupsWithCheckOnlineOption(
    HANDLE  hLsaConnection,
    DWORD   dwGroupInfoLevel,
    DWORD   dwMaxNumGroups,
    BOOLEAN bCheckGroupMembersOnline,
    LSA_FIND_FLAGS FindFlags,
    PHANDLE phResume
    );

/**
 * @ingroup group
 * @brief Retrieve next list of groups during enumeration
 *
 * Retrieves the next list of groups for an in-progress enumeration.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] hResume the enumeration handle
 * @param[out] pdsNumGroupsFound the number of groups returned
 * @param[out] pppGroupInfoList a heap-allocated list of group info structures
 * of the level specified in the call to #LsaBeginEnumGroups().  It should be
 * freed with #LsaFreeGroupInfoList().
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaEnumGroups(
    HANDLE  hLsaConnection,
    HANDLE  hResume,
    PDWORD  pdwNumGroupsFound,
    PVOID** pppGroupInfoList
    );

/**
 * @ingroup group
 * @brief End group enumeration
 *
 * Ends a group enumeration, releasing any associated resources.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in,out] hResume the enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaEndEnumGroups(
    HANDLE  hLsaConnection,
    HANDLE  hResume
    );

/**
 * @ingroup group
 * @brief Free a list of group info structures
 *
 * Frees a list of group info structures of the specified level.
 *
 * @param[in] dwLevel the info level of the structures
 * @param[in,out] pGroupInfoList the info list
 * @param[dwNumGroups] dwNumGroups the number of elements in the list
 */
VOID
LsaFreeGroupInfoList(
    DWORD  dwLevel,
    PVOID* pGroupInfoList,
    DWORD  dwNumGroups
    );

/**
 * @ingroup group
 * @brief Free a group info structure
 *
 * Frees a single group info structure of the specified level.
 *
 * @param[in] dwLevel the info level of the structures
 * @param[in,out] pGroupInfo the info structure
 */
VOID
LsaFreeGroupInfo(
    DWORD dwLevel,
    PVOID pGroupInfo
    );

/* FIXME: should these be public? */
#ifndef DOXYGEN

VOID
LsaFreeEnumObjectsInfo(
    PLSA_ENUM_OBJECTS_INFO pInfo
    );

VOID
LsaFreeNSSArtefactInfoList(
    DWORD  dwLevel,
    PVOID* pNSSArtefactInfoList,
    DWORD  dwNumNSSArtefacts
    );
#endif

VOID
LsaFreeNSSArtefactInfo(
    DWORD  dwLevel,
    PVOID  pNSSArtefactInfo
    );

DWORD
LsaAddUser(
    HANDLE hLsaConnection,
    PVOID  pUserInfo,
    DWORD  dwUserInfoLevel
    );

DWORD
LsaModifyUser(
    HANDLE hLsaConnection,
    PLSA_USER_MOD_INFO pUserModInfo
    );

DWORD
LsaDeleteUserById(
    HANDLE hLsaConnection,
    uid_t  uid
    );

DWORD
LsaDeleteUserByName(
    HANDLE hLsaConnection,
    PCSTR  pszName
    );

/**
 * @ingroup user
 * @brief Look up user by name
 *
 * Looks up information on a user by its name.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] pszGroupName the name of the user
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwUserInfoLevel the desired info level for the returned user info structure
 * @param[out] ppGroupInfo a heap-allocated group info structure for the found group
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_GROUP the specified name did not match any known group
 */
DWORD
LsaFindUserByName(
    HANDLE hLsaConnection,
    PCSTR  pszName,
    DWORD  dwUserInfoLevel,
    PVOID* ppUserInfo
    );

/**
 * @ingroup user
 * @brief Look up user by ID
 *
 * Looks up information on a user by its user ID.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] uid the user ID of the user
 * @param[in] FindFlags options for the lookup operation
 * @param[in] dwUserInfoLevel the desired info level for the returned user info structure
 * @param[out] ppUserInfo a heap-allocated user info structure for the found user
 * @retval LSA_ERROR_SUCCESS success
 * @retval LSA_ERROR_NO_SUCH_USER the specified user ID did not match any known user
 */
DWORD
LsaFindUserById(
    HANDLE hLsaConnection,
    uid_t  uid,
    DWORD  dwUserInfoLevel,
    PVOID* ppUserInfo
    );

DWORD
LsaGetNamesBySidList(
    IN HANDLE hLsaConnection,
    IN size_t sCount,
    IN PSTR* ppszSidList,
    OUT PLSA_SID_INFO* ppSIDInfoList,
    OUT OPTIONAL CHAR *pchDomainSeparator
    );

VOID
LsaFreeSIDInfoList(
    PLSA_SID_INFO  ppSIDInfoList,
    size_t         stNumSID
    );

VOID
LsaFreeSIDInfo(
    PLSA_SID_INFO pSIDInfo
    );

/**
 * @ingroup user
 * @brief Begin user enumeration
 *
 * Begins an enumeration of all known users.  This function returns an
 * enumeration handle which can be used with #LsaEnumUsers() to fetch
 * lists of users in increments of up to dwMaxNumUsers.
 *
 * You must call #LsaEndEnumUsers() on the enumeration handle when
 * finished with the enumeration.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in] dwUserInfoLevel the desired info level for the user info structures
 * returned during the enumeration
 * @param[in] dwMaxNumUsers the maximum number of user info structures to
 * return in each subsequent call to #LsaEnumUsers()
 * @param[in] FindFlags options for the lookup operation
 * @param[out] phResume the created enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaBeginEnumUsers(
    HANDLE  hLsaConnection,
    DWORD   dwUserInfoLevel,
    DWORD   dwMaxNumUsers,
    LSA_FIND_FLAGS FindFlags,
    PHANDLE phResume
    );

/**
 * @ingroup user
 * @brief End user enumeration
 *
 * Ends a user enumeration, releasing any associated resources.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in,out] hResume the enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaEnumUsers(
    HANDLE  hLsaConnection,
    HANDLE  hResume,
    PDWORD  pdwNumUsersFound,
    PVOID** pppUserInfoList
    );

/**
 * @ingroup user
 * @brief End user enumeration
 *
 * Ends a user enumeration, releasing any associated resources.
 *
 * @param[in] hLsaConnection the connection handle
 * @param[in,out] hResume the enumeration handle
 * @retval #LSA_ERROR_SUCCESS success
 */
DWORD
LsaEndEnumUsers(
    HANDLE hLsaConnection,
    HANDLE hResume
    );

/**
 * @ingroup user
 * @brief Free a list of user info structures
 *
 * Frees a list of user info structures of the specified level.
 *
 * @param[in] dwLevel the info level of the structures
 * @param[in,out] pUserInfoList the info list
 * @param[dwNumUsers] dwNumUsers the number of elements in the list
 */
VOID
LsaFreeUserInfoList(
    DWORD  dwLevel,
    PVOID* pUserInfoList,
    DWORD  dwNumUsers
    );


/**
 * @ingroup user
 * @brief Free a user info structure
 *
 * Frees a single user info structure of the specified level.
 *
 * @param[in] dwLevel the info level of the structures
 * @param[in,out] pUserInfo the info structure
 */
VOID
LsaFreeUserInfo(
    DWORD dwLevel,
    PVOID pUserInfo
    );

DWORD
LsaAuthenticateUser(
    HANDLE hLsaConnection,
    PCSTR  pszLoginName,
    PCSTR  pszPassword
    );

DWORD
LsaFreeAuthUserInfo(
	PLSA_AUTH_USER_INFO *ppAuthUserInfo
	);

DWORD
LsaFreeAuthUserParams(
	PLSA_AUTH_USER_PARAMS *ppAuthUserParams
	);

DWORD
LsaAuthenticateUserEx(
	IN HANDLE hLsaConnection,
	IN LSA_AUTH_USER_PARAMS* pParams,
	OUT PLSA_AUTH_USER_INFO* ppUserInfo
	);

DWORD
LsaValidateUser(
    HANDLE hLsaConnection,
    PCSTR  pszLoginName,
    PCSTR  pszPassword
    );

DWORD
LsaCheckUserInList(
    HANDLE   hLsaConnection,
    PCSTR    pszLoginName,
    PCSTR    pszListName
    );

DWORD
LsaChangePassword(
    HANDLE hLsaConnection,
    PCSTR  pszLoginName,
    PCSTR  pszNewPassword,
    PCSTR  pszOldPassword
    );

DWORD
LsaSetPassword(
    HANDLE hLsaConnection,
    PCSTR  pszLoginName,
    PCSTR  pszNewPassword
    );

DWORD
LsaOpenSession(
    HANDLE hLsaConnection,
    PCSTR  pszLoginId
    );

DWORD
LsaCloseSession(
    HANDLE hLsaConnection,
    PCSTR  pszLoginId
    );

DWORD
LsaGetMetrics(
    HANDLE hLsaConnection,
    DWORD  dwInfoLevel,
    PVOID* ppMetricPack
    );

DWORD
LsaGetStatus(
   HANDLE      hLsaConnection,
   PLSASTATUS* ppLsaStatus
   );

DWORD
LsaRefreshConfiguration(
   HANDLE      hLsaConnection
   );

VOID
LsaFreeStatus(
   PLSASTATUS pLsaStatus
   );

/**
 * @ingroup connection
 * @brief Closes connection to LSASS server
 *
 * Closes a connection handle opened with #LsaOpenServer().
 *
 * @param[in,out] hConnection the connection handle to close
 * @retval LSA_ERROR_SUCCESS success
 * @retval EINVAL the handle was invalid
 */
DWORD
LsaCloseServer(
    HANDLE hConnection
    );

/**
 * @ingroup utility
 * @brief Get error code description
 *
 * Retrives a human-readable description of an LSASS
 * error code.  The description will be copied into
 * the specified buffer, but will be truncated if it
 * does not fit.
 *
 * @param[in] dwError the error code
 * @param[out] pszBuffer the buffer into which the description will be copied
 * @param[in] stBufSize the size of the provided buffer
 * @return the full size of the error string, which may be larger than the
 * provided buffer
 */
 size_t
LsaGetErrorString(
    DWORD  dwError,
    PSTR   pszBuffer,
    size_t stBufSize
    );

/**
 * @ingroup utility
 * @brief Free a memory block
 *
 * Frees a single memory block allocated by an LSASS client API
 * function.  This may be used to peform fine-grained management
 * of heap-allocated structures in lieu of functions such as
 * #LsaFreeGroupInfo().
 *
 * @param[in,out] pMemory a pointer to the memory block
 */
VOID
LsaFreeMemory(
    PVOID pMemory
    );

DWORD
LsaGetErrorMessageForLoggingEvent(
    DWORD dwError,
    PSTR* ppszErrorMsg
    );


/*
 * LSA_DATA_BLOB access functions and methods
 */

DWORD
LsaDataBlobAllocate(
	PLSA_DATA_BLOB *ppBlob,
	DWORD dwSize
	);
DWORD
LsaDataBlobFree(
	PLSA_DATA_BLOB *ppBlob
	);

DWORD
LsaDataBlobStore(
	PLSA_DATA_BLOB *ppBlob,
	DWORD dwSize,
	const PBYTE pBuffer
	);

DWORD
LsaDataBlobCopy(
	PLSA_DATA_BLOB *ppDst,
	PLSA_DATA_BLOB pSrc
	);

DWORD
LsaDataBlobLength(
	PLSA_DATA_BLOB pBlob
	);

PBYTE
LsaDataBlobBuffer(
	PLSA_DATA_BLOB pBlob
	);

#endif /* __LSA_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
