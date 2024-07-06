#pragma once

#define SPN_ERRNO_BASE (0xF000)

#define SPN_OK (0)
#define SPN_EAGAIN (SPN_ERRNO_BASE + 1)
#define SPN_EIO (SPN_ERRNO_BASE + 2)
#define SPN_EEXIST (SPN_ERRNO_BASE + 3)
#define SPN_EINVAL (SPN_ERRNO_BASE + 4)