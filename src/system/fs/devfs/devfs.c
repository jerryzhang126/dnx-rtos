                gid_t                     gid;
                uid_t                     uid;
                mode_t                    mode;
        UNUSED_ARG(src_path);
 * @param[in]            flags                  file open flags
API_FS_OPEN(devfs, void *fs_handle, void **extra, fd_t *fd, fpos_t *fpos, const char *path, vfs_open_flags_t flags)
        UNUSED_ARG(fd);
                                open = driver_open(node->IF.drv, flags);
API_FS_WRITE(devfs, void *fs_handle,void *extra, fd_t fd, const u8_t *src, size_t count, fpos_t *fpos, struct vfs_fattr fattr)
        UNUSED_ARG(fs_handle);
API_FS_READ(devfs, void *fs_handle, void *extra, fd_t fd, u8_t *dst, size_t count, fpos_t *fpos, struct vfs_fattr fattr)
        UNUSED_ARG(fs_handle);
        UNUSED_ARG(fs_handle);
        } else if (node->type == FILE_TYPE_PIPE && request == IOCTL_PIPE__CLOSE) {
        UNUSED_ARG(fs_handle);
        UNUSED_ARG(fs_handle);
        UNUSED_ARG(fs_handle);
        UNUSED_ARG(path);
        UNUSED_ARG(fs_handle);
        UNUSED_ARG(dir);
API_FS_CHMOD(devfs, void *fs_handle, const char *path, mode_t mode)
API_FS_CHOWN(devfs, void *fs_handle, const char *path, uid_t owner, gid_t group)
//==============================================================================
/**
 * @brief Synchronize all buffers to a medium
 *
 * @param[in ]          *fs_handle              file system allocated memory
 *
 * @return None
 */
//==============================================================================
API_FS_SYNC(devfs, void *fs_handle)
{
        UNUSED_ARG(fs_handle);
}
