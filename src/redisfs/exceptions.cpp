#include "redisfs/exceptions.h"

redisfs::RedisFSError::RedisFSError( const std::string & what ) : std::runtime_error( what ) {}

redisfs::CorruptFilesystemError::CorruptFilesystemError( const std::string & what  ) : RedisFSError( what ) {}