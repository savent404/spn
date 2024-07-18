#pragma once

#define SPN_OK 0                   /* */
#define SPN_EPERM 1                /* Operation not permitted */
#define SPN_ENOENT 2               /* No such file or directory */
#define SPN_ESRCH 3                /* No such process */
#define SPN_EINTR 4                /* Interrupted system call */
#define SPN_EIO 5                  /* I/O error */
#define SPN_ENXIO 6                /* No such device or address */
#define SPN_E2BIG 7                /* Arg list too long */
#define SPN_ENOEXEC 8              /* Exec format error */
#define SPN_EBADF 9                /* Bad file number */
#define SPN_ECHILD 10              /* No child processes */
#define SPN_EAGAIN 11              /* Try again */
#define SPN_ENOMEM 12              /* Out of memory */
#define SPN_EACCES 13              /* Permission denied */
#define SPN_EFAULT 14              /* Bad address */
#define SPN_ENOTBLK 15             /* Block device required */
#define SPN_EBUSY 16               /* Device or resource busy */
#define SPN_EEXIST 17              /* File exists */
#define SPN_EXDEV 18               /* Cross-device link */
#define SPN_ENODEV 19              /* No such device */
#define SPN_ENOTDIR 20             /* Not a directory */
#define SPN_EISDIR 21              /* Is a directory */
#define SPN_EINVAL 22              /* Invalid argument */
#define SPN_ENFILE 23              /* File table overflow */
#define SPN_EMFILE 24              /* Too many open files */
#define SPN_ENOTTY 25              /* Not a typewriter */
#define SPN_ETXTBSY 26             /* Text file busy */
#define SPN_EFBIG 27               /* File too large */
#define SPN_ENOSPC 28              /* No space left on device */
#define SPN_ESPIPE 29              /* Illegal seek */
#define SPN_EROFS 30               /* Read-only file system */
#define SPN_EMLINK 31              /* Too many links */
#define SPN_EPIPE 32               /* Broken pipe */
#define SPN_EDOM 33                /* Math argument out of domain of func */
#define SPN_ERANGE 34              /* Math result not representable */
#define SPN_EDEADLK 35             /* Resource deadlock would occur */
#define SPN_ENAMETOOLONG 36        /* File name too long */
#define SPN_ENOLCK 37              /* No record locks available */
#define SPN_ENOSYS 38              /* Function not implemented */
#define SPN_ENOTEMPTY 39           /* Directory not empty */
#define SPN_ELOOP 40               /* Too many symbolic links encountered */
#define SPN_EWOULDBLOCK SPN_EAGAIN /* Operation would block */
#define SPN_ENOMSG 42              /* No message of desired type */
#define SPN_EIDRM 43               /* Identifier removed */
#define SPN_ECHRNG 44              /* Channel number out of range */
#define SPN_EL2NSYNC 45            /* Level 2 not synchronized */
#define SPN_EL3HLT 46              /* Level 3 halted */
#define SPN_EL3RST 47              /* Level 3 reset */
#define SPN_ELNRNG 48              /* Link number out of range */
#define SPN_EUNATCH 49             /* Protocol driver not attached */
#define SPN_ENOCSI 50              /* No CSI structure available */
#define SPN_EL2HLT 51              /* Level 2 halted */
#define SPN_EBADE 52               /* Invalid exchange */
#define SPN_EBADR 53               /* Invalid request descriptor */
#define SPN_EXFULL 54              /* Exchange full */
#define SPN_ENOANO 55              /* No anode */
#define SPN_EBADRQC 56             /* Invalid request code */
#define SPN_EBADSLT 57             /* Invalid slot */

#define SPN_EDEADLOCK SPN_EDEADLK

#define SPN_EBFONT 59          /* Bad font file format */
#define SPN_ENOSTR 60          /* Device not a stream */
#define SPN_ENODATA 61         /* No data available */
#define SPN_ETIME 62           /* Timer expired */
#define SPN_ENOSR 63           /* Out of streams resources */
#define SPN_ENONET 64          /* Machine is not on the network */
#define SPN_ENOPKG 65          /* Package not installed */
#define SPN_EREMOTE 66         /* Object is remote */
#define SPN_ENOLINK 67         /* Link has been severed */
#define SPN_EADV 68            /* Advertise error */
#define SPN_ESRMNT 69          /* Srmount error */
#define SPN_ECOMM 70           /* Communication error on send */
#define SPN_EPROTO 71          /* Protocol error */
#define SPN_EMULTIHOP 72       /* Multihop attempted */
#define SPN_EDOTDOT 73         /* RFS specific error */
#define SPN_EBADMSG 74         /* Not a data message */
#define SPN_EOVERFLOW 75       /* Value too large for defined data type */
#define SPN_ENOTUNIQ 76        /* Name not unique on network */
#define SPN_EBADFD 77          /* File descriptor in bad state */
#define SPN_EREMCHG 78         /* Remote address changed */
#define SPN_ELIBACC 79         /* Can not access a needed shared library */
#define SPN_ELIBBAD 80         /* Accessing a corrupted shared library */
#define SPN_ELIBSCN 81         /* .lib section in a.out corrupted */
#define SPN_ELIBMAX 82         /* Attempting to link in too many shared libraries */
#define SPN_ELIBEXEC 83        /* Cannot exec a shared library directly */
#define SPN_EILSEQ 84          /* Illegal byte sequence */
#define SPN_ERESTART 85        /* Interrupted system call should be restarted */
#define SPN_ESTRPIPE 86        /* Streams pipe error */
#define SPN_EUSERS 87          /* Too many users */
#define SPN_ENOTSOCK 88        /* Socket operation on non-socket */
#define SPN_EDESTADDRREQ 89    /* Destination address required */
#define SPN_EMSGSIZE 90        /* Message too long */
#define SPN_EPROTOTYPE 91      /* Protocol wrong type for socket */
#define SPN_ENOPROTOOPT 92     /* Protocol not available */
#define SPN_EPROTONOSUPPORT 93 /* Protocol not supported */
#define SPN_ESOCKTNOSUPPORT 94 /* Socket type not supported */
#define SPN_EOPNOTSUPP 95      /* Operation not supported on transport endpoint */
#define SPN_EPFNOSUPPORT 96    /* Protocol family not supported */
#define SPN_EAFNOSUPPORT 97    /* Address family not supported by protocol */
#define SPN_EADDRINUSE 98      /* Address already in use */
#define SPN_EADDRNOTAVAIL 99   /* Cannot assign requested address */
#define SPN_ENETDOWN 100       /* Network is down */
#define SPN_ENETUNREACH 101    /* Network is unreachable */
#define SPN_ENETRESET 102      /* Network dropped connection because of reset */
#define SPN_ECONNABORTED 103   /* Software caused connection abort */
#define SPN_ECONNRESET 104     /* Connection reset by peer */
#define SPN_ENOBUFS 105        /* No buffer space available */
#define SPN_EISCONN 106        /* Transport endpoint is already connected */
#define SPN_ENOTCONN 107       /* Transport endpoint is not connected */
#define SPN_ESHUTDOWN 108      /* Cannot send after transport endpoint shutdown */
#define SPN_ETOOMANYREFS 109   /* Too many references: cannot splice */
#define SPN_ETIMEDOUT 110      /* Connection timed out */
#define SPN_ECONNREFUSED 111   /* Connection refused */
#define SPN_EHOSTDOWN 112      /* Host is down */
#define SPN_EHOSTUNREACH 113   /* No route to host */
#define SPN_EALREADY 114       /* Operation already in progress */
#define SPN_EINPROGRESS 115    /* Operation now in progress */
#define SPN_ESTALE 116         /* Stale NFS file handle */
#define SPN_EUCLEAN 117        /* Structure needs cleaning */
#define SPN_ENOTNAM 118        /* Not a XENIX named type file */
#define SPN_ENAVAIL 119        /* No XENIX semaphores available */
#define SPN_EISNAM 120         /* Is a named type file */
#define SPN_EREMOTEIO 121      /* Remote I/O error */
#define SPN_EDQUOT 122         /* Quota exceeded */

#define SPN_ENOMEDIUM 123   /* No medium found */
#define SPN_EMEDIUMTYPE 124 /* Wrong medium type */
