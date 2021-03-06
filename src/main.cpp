#define FUSE_USE_VERSION 31

#include <memory>
#include <fuse3/fuse.h>

#include <sw/redis++/redis++.h>
#include <sw/redis++/redis_cluster.h>

#include "redisfs/redisfs.h"
#include "redisfs/redis.h"
#include "redisfs/utils.hpp"

constexpr size_t BLOCK_SIZE = 512 * redisfs::Size<size_t>::MEGA;

namespace redisfs {

    namespace fuse {

        static struct redis_fs_info {

            std::shared_ptr<RedisFS> fs;

        } redis_fs_info;

        //required fuse functions

        static void * init( struct fuse_conn_info * conn, struct fuse_config * cfg ) {

            //TODO
            //(void) conn;
            //cfg->kernel_cache = 0;

            std::string host = "tcp://127.0.0.1"; // Required.
            int         port = 7000;        // Optional. The default port is 6379.
            const std::string & uri = host + ":" + std::to_string( port );

            const sw::redis::ConnectionOptions connectionOptions( uri );
            //std::shared_ptr<KVStore> store = std::make_shared<redis::RedisClusterStore>( connectionOptions );
            std::shared_ptr<KVStore> store = std::make_shared<MemoryStore>();

            redis_fs_info.fs = std::make_shared<RedisFS>( store, BLOCK_SIZE );

            return ( void * ) &redis_fs_info;
            
        }

        static int create(const char * path, mode_t mode, struct fuse_file_info *){
            return redis_fs_info.fs->create(path, mode);
        }

        static int access(const char *path, int mode) {
            return redis_fs_info.fs->access(path, mode);
        }


        static int getattr( const char * path, struct stat * stbuf,
                            struct fuse_file_info * fi ) {

            return redis_fs_info.fs->getattr( path, stbuf );

        }

        static int readdir( const char * path, void * buf, fuse_fill_dir_t filler, off_t offset, 
                            struct fuse_file_info * fi, enum fuse_readdir_flags flags ) {

            ( void ) offset;
            ( void ) fi;

            return redis_fs_info.fs->readdir( path, buf, filler, offset );

        }

        static int open( const char * path, struct fuse_file_info * fi ) {

            return redis_fs_info.fs->open( path );
            
        }

        static int read( const char * path, char * buf, size_t size, off_t offset,
                        struct fuse_file_info * fi ) {

            ( void ) fi;
            return redis_fs_info.fs->read( path, buf, size, offset );
            
        }

        static int write( const char * path, const char * buf, size_t size, off_t offset, 
                            struct fuse_file_info * fi ) {

            return redis_fs_info.fs->write( path, buf, size, offset );

        }

        int release( const char * path, struct fuse_file_info * fi ) {

            return redis_fs_info.fs->release( path );

        }

        int utimens(const char * path , const struct timespec tv[2], struct fuse_file_info *fi){
            return redis_fs_info.fs->utimens(path, tv);
        }


        static const struct fuse_operations test_oper {
            .getattr = getattr,
            .open = open,
            .read = read,
            .write = write,
            .release = release,
            .readdir = readdir,
            .init = init,
            .access = access,
            .create = create,
            .utimens = utimens,
        };

    }

}

int main( int argc, char *argv[] ) {

    return fuse_main( argc, argv, &redisfs::fuse::test_oper, NULL );

}
