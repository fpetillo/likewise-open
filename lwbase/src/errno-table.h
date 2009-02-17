/*
 * Copyright (c) Likewise Software.  All rights Reserved.
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
 * license@likewise.com
 */

/*
 * Module Name:
 *
 *        errno.c
 *
 * Abstract:
 *
 *        UNIX errno code table
 *
 * Authors: Brian Koropoff (bkoropoff@likewise.com)
 *
 */

#define S ERRNO_CODE

S ( 0, STATUS_SUCCESS)
S ( E2BIG, -1 )
S ( EACCES, STATUS_ACCESS_DENIED )
S ( EADDRINUSE, -1 )
S ( EADDRNOTAVAIL, -1 )
S ( EAFNOSUPPORT, -1 )
S ( EAGAIN, -1 )
S ( EALREADY, -1 )
S ( EBADF, STATUS_INVALID_HANDLE )
S ( EBADMSG, STATUS_INVALID_NETWORK_RESPONSE )
S ( EBUSY, -1 )
S ( ECANCELED, -1 )
S ( ECHILD, -1 )
S ( ECONNABORTED, -1 )
S ( ECONNREFUSED, -1 )
S ( ECONNRESET, -1 )
S ( EDEADLK, -1 )
S ( EDESTADDRREQ, -1 )
S ( EDOM, -1 )
S ( EDQUOT, -1 )
S ( EEXIST, STATUS_OBJECT_NAME_COLLISION )
S ( EFAULT, -1 )
S ( EFBIG, -1 )
S ( EHOSTUNREACH, -1 )
S ( EIDRM, -1 )
S ( EILSEQ, -1 )
S ( EINPROGRESS, -1 )
S ( EINTR, -1 )
S ( EINVAL, STATUS_INVALID_PARAMETER )
S ( EIO, STATUS_UNEXPECTED_IO_ERROR )
S ( EISCONN, -1 )
S ( EISDIR, -1 )
S ( ELOOP, -1 )
S ( EMFILE, -1 )
S ( EMLINK, -1 )
S ( EMSGSIZE, -1 )
S ( EMULTIHOP, -1 )
S ( ENAMETOOLONG, -1 )
S ( ENETDOWN, -1 )
S ( ENETRESET, -1 )
S ( ENETUNREACH, -1 )
S ( ENFILE, -1 )
S ( ENOBUFS, -1 )
#if defined(ENODATA)
S ( ENODATA, -1 )
#endif
S ( ENODEV, -1 )
S ( ENOENT, STATUS_OBJECT_NAME_NOT_FOUND )
S ( ENOEXEC, -1 )
S ( ENOLCK, -1 )
S ( ENOLINK, -1 )
S ( ENOMEM, STATUS_INSUFFICIENT_RESOURCES )
S ( ENOMSG, -1 )
S ( ENOPROTOOPT, -1 )
S ( ENOSPC, STATUS_DISK_FULL )
#if defined(ENOSR)
S ( ENOSR, -1 )
#endif
#if defined(ENOSTR)
S ( ENOSTR, -1 )
#endif
S ( ENOSYS, STATUS_NOT_SUPPORTED )
S ( ENOTCONN, -1 )
S ( ENOTDIR, -1 )
S ( ENOTEMPTY, -1 )
S ( ENOTSOCK, -1 )
S ( ENOTSUP, -1 )
S ( ENOTTY, -1 )
S ( ENXIO, -1 )
S ( EOPNOTSUPP, -1 )
S ( EOVERFLOW, -1 )
S ( EPERM, STATUS_ACCESS_DENIED )
S ( EPIPE, -1 )
S ( EPROTO, -1 )
S ( EPROTONOSUPPORT, -1 )
S ( EPROTOTYPE, -1 )
S ( ERANGE, -1 )
S ( EROFS, -1 )
S ( ESPIPE, -1 )
S ( ESRCH, STATUS_NOT_FOUND )
S ( ESTALE, -1 )
#if defined(ETIME)
S ( ETIME, -1 )
#endif
S ( ETIMEDOUT, STATUS_IO_TIMEOUT )
S ( ETXTBSY, -1 )
S ( EWOULDBLOCK, -1 )
S ( EXDEV, -1 )

#undef S
