#pragma once

#if defined(TESTING)
#define PRIVATE_UNLESS_TESTING public:
#else
#define PRIVATE_UNLESS_TESTING private:
#endif
