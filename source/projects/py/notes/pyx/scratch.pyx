
# Path wrongly comnines all Path functyions with sysfile funcs.
# How to separate them to make sense??

"""
cdef class Path:
    
    # path funcs
    def __cinit__(self)
    def __init__(self, str filename)
    def type(self)
    def creator(self)
    def flags(self)
    def set_default_path(self, short path_id, bint recursive = False)
    def to_absolute_path(self, str filename, short path_id) -> str
    def to_pathname(self, str filename, int path_id)
    def from_pathname(self, str pathname) -> tuple[str, int]
    def create_file(self, str filename, short path_id) -> int
    def locate(self, str filename, str code = 'TEXT') -> str
    def _get_info(self)
    def rename(self, str new_name)
    def delete(self)

    # path funcs with file handler features
    def open_sysfile(self, str filename, short path_id, str perm = 'w')
    def create_sysfile(self, str filename, short path_id, str fourchar_type)

    def close_sysfile(self)
    def sysfile_read(self, mx.t_ptr_size count) -> str
    def sysfile_readtohandle(self, int size) -> list[str]
    def sysfile_readtoptr(self, int bufsize) -> str
    def sysfile_write(self, object contents)
    def sysfile_seteof(self, int nbytes)
    def sysfile_geteof(self) -> int
    def sysfile_getpos(self) -> int
    cdef mx.t_max_err sysfile_spoolcopy(self, mx.t_filehandle src, mx.t_filehandle dst, mx.t_ptr_size size)
    def sysfile_readtextfile(self, int maxlen = 0, int flags = 0) -> str
    def sysfile_writetextfile(self, str text)
    cdef mx.t_max_err sysfile_openhandle(self, char **h, mx.t_sysfile_flags flags, mx.t_filehandle *fh)
    cdef mx.t_max_err sysfile_openptrsize(self, char *p, mx.t_ptr_size length, mx.t_sysfile_flags flags, mx.t_filehandle *fh)


    

Path
FileHandle
File



"""

cdef class Path:
    """A wrapper for Max friendly paths"""

    cdef public str filename    # name used to search for file
    cdef public str pathname    # absolute path after file is found
    cdef short path_id          #  max short code for parent folder
    cdef mx.t_fileinfo info     # instance of file metadata struct
    cdef mx.t_filehandle fh     # file handle
    cdef int size               # size of file handle file
    cdef mx.t_ptr_uint filedate # modificiation date

    def __cinit__(self):
        self.path_id = 0
        self.filedate = 0
        self.fh = NULL


    def __init__(self, str filename):
        self.filename = filename
        self.pathname = self.locate(filename)
        self._get_info() # populate info attribute

    @property
    def type(self):
        """type (four-char-code)"""
        return int_to_fourchar(self.info.type)

    @property
    def creator(self):
        """Mac-only creator (four-char-code)"""
        return int_to_fourchar(self.info.creator)

    @property
    def flags(self):
        """One of the values defined in e_max_fileinfo_flags"""
        return self.info.flags

    def set_default_path(self, short path_id, bint recursive = False):
        """Install a path as the default search path.
        
        The default path is searched before the Max search path. For instance, 
        when loading a patcher from a directory outside the search path, the 
        patcher's directory is searched for files before the search path. 
        path_setdefault() allows you to set a path as the default.
        """
        mx.path_setdefault(path_id, <short>recursive)

    def to_absolute_path(self, str filename, short path_id) -> str:
        """Translates a Max path+filename combo into a correct POSIX absolute path

        The resulting absolute path can be used to pass to libraries
        and also handles multiple volumes correctly.
        """
        cdef char pathname[2048]    # absolute path result of search
        cdef mx.t_max_err err = mx.path_toabsolutesystempath(self.path_id,
            filename.encode(), pathname)
        if err != mx.MAX_ERR_NONE:
            raise IOError(f"can't convert {filename} to absolute path")
        return pathname.decode()

    def to_pathname(self, str filename, int path_id):
        """Create a fully qualified file name from a Path ID/file name combination.

        This routine will only convert a pathname pair to a valid path
        string if the path exists.
        """
        cdef char* pathname = NULL
        cdef short res = mx.path_topathname(path_id, filename.encode(), pathname)
        if res != 0:
            raise IOError("could not set filename and path_id")
        return pathname.decode()

    def from_pathname(self, str pathname) -> tuple[str, int]:
        """Create and return a filename and Path ID combination from a fully
        qualified file name.

        Note that path_frompathname() does not require that the file actually exist. 
        In this way you can use it to convert a full path you may have received as an 
        argument to a file writing message to a form appropriate to provide to 
        a routine such as path_createfile().
        """
        cdef short path_id = 0
        cdef char* filename = NULL
        cdef short res = mx.path_frompathname(pathname.encode(), &path_id, filename)
        if res != 0:
            raise IOError("could not get filename and path_id from pathname")
        return (filename.encode(), path_id)

    def create_file(self, str filename, short path_id) -> int:
        """create file from a filename and path_id"""
        cdef short new_path_id = 0
        cdef short res = mx.path_createfolder(path_id, filename.encode(), &new_path_id)
        return new_path_id

    def locate(self, str filename, str code = 'TEXT') -> str:
        """Find a file by name.

        If a complete path is not specified, search for the name in the search path.
        """
        cdef char pathname[2048]    # absolute path result of search
        cdef mx.t_fourcc filetype = fourchar_to_int(code)
        cdef mx.t_fourcc outtype  = 0
        cdef mx.t_max_err err = mx.MAX_ERR_NONE

        cdef short res = mx.locatefile_extended(filename.encode(),
            &self.path_id, &outtype, &filetype, 1)
        if res != 0:
            raise IOError(f"can't find file '{filename}'")

        return self.to_absolute_path(filename, self.path_id)

    def _get_info(self):
        """populate t_fileinfo struct file metatadata  instance"""
        cdef short res = mx.path_fileinfo(self.filename.encode(), self.path_id, &self.info)
        if res != 0:
            raise IOError(f"couldn't retrieve file info for {self.filename}")

    def rename(self, str new_name):
        """Rename the file"""
        cdef short res = mx.path_renamefile(self.filename.encode, self.path_id, new_name.encode())
        if res != 0:
            raise IOError(f"couldn't rename {self.filename}")

    def delete(self):
        """Delete located file."""
        cdef short res = mx.path_deletefile(self.filename.encode(), self.path_id)
        if res != 0:
            raise IOError(f"couldn't delete {self.filename}")


    def open_sysfile(self, str filename, short path_id, str perm = 'w'):
        """Open a file given a filename and Path ID.
        
        Will update the t_filehandle reference in the object to point to the open file

        permission modes are:
            'r': 1 read
            'w': 2 write
           'rw': 3 read/write 
        """
        cdef short _perm = <short>dict(r=1,w=2,rw=3)[perm]
        cdef short res = mx.path_opensysfile(filename.encode(), path_id, &self.fh, _perm)
        if res != 0:
            raise IOError(f"could not open sysfile {filename} with path_id={path_id}")   

    def create_sysfile(self, str filename, short path_id, str fourchar_type):
        """Create a file given a type code, a filename, and a Path ID.
        
        Will update the t_filehandle reference in the object to point to the created file

        permission modes are:
            'r': 1 read
            'w': 2 write
           'rw': 3 read/write 
        """
        cdef mx.t_fourcc fctype = <mx.t_fourcc>fourchar_to_int(fourchar_type)
        cdef short res = mx.path_createsysfile(filename.encode(), path_id, fctype, &self.fh)
        if res != 0:
            raise IOError(f"could not create sysfile {filename} "
                          f"with path_id={path_id}, type={fourchar_type}")

    def close_sysfile(self):
        """Close a file opened with sysfile_open().

        This function is similar to FSClose() or fclose(). 
        It should be used instead of system-specific file closing routines in order to make max external 
        code that will compile cross-platform.
        """
        cdef mx.t_max_err err = mx.sysfile_close(self.fh)
        if err != mx.MAX_ERR_NONE:
            raise IOError(f"can't close open filehandle")

    def sysfile_read(self, mx.t_ptr_size count) -> str:
        """Read a file from disk.

        This function is similar to FSRead() or fread(). It should be used instead of 
        these functions (or other system-specific file reading routines) in order 
        to make max external code that will compile cross-platform. It reads 
        "count" bytes from file handle at current file position into "bufptr". 
        The byte count actually read is set in "count", and the file position is 
        updated by the actual byte count read.
        """
        cdef char * bufptr = <char *>mx.sysmem_newptr(count * sizeof(char))
        cdef mx.t_max_err err = mx.sysfile_read(self.fh, &count, <char *>bufptr)
        if err != mx.MAX_ERR_NONE:
            raise IOError("could not read contents of from from disk")
        cdef str result = bufptr.decode()
        mx.sysmem_freeptr(bufptr)
        return result

    def sysfile_readtohandle(self, int size) -> list[str]:
        """Read the contents of a file into a handle."""
        cdef int i = 0
        cdef char** fh = <mx.t_handle>mx.sysmem_newhandleclear(<mx.t_ptr_size>size)
        cdef mx.t_max_err err = mx.sysfile_readtohandle(self.fh, &fh)
        if err != mx.MAX_ERR_NONE:
            raise IOError("could not read contents of filehandle into a handle")
        lines = []
        for i in range(size):
            line = fh[i].decode()
            # line = fh[i][0].decode()
            lines.append(line)
        mx.sysmem_freehandle(fh)
        return lines

    def sysfile_readtoptr(self, int bufsize) -> str:
        """Read the contents of a file into a pointer."""
        cdef char* bufptr = <char *>mx.sysmem_newptr(bufsize * sizeof(char))
        cdef mx.t_max_err err = mx.sysfile_readtoptr(self.fh, &bufptr)
        if err != mx.MAX_ERR_NONE:
            raise IOError("could not read contents of a file into a pointer")
        cdef str result = bufptr.decode()
        mx.sysmem_freeptr(bufptr)

    def sysfile_write(self, object contents):
        """Write part of a file to disk.

        This function is similar to FSWrite() or fwrite(). It should be used instead 
        of these functions (or other system-specific file reading routines) in 
        order to make max external code that will compile cross-platform. The 
        function writes "count" bytes from "bufptr" into file handle at current 
        file position. The byte count actually written is set in "count", and the
        file position is updated by the actual byte count written.
        """
        cdef mx.t_ptr_size count = <mx.t_ptr_size>len(contents)
        cdef char* bufptr = <char *>mx.sysmem_newptr(count * sizeof(char))
        cdef mx.t_max_err err = mx.MAX_ERR_NONE
        if isinstance(contents, bytes):
            bufptr = contents
        elif isinstance(contents, str):
            bufptr = <bytes>contents
        else:
            raise TypeError("could not write this type file handler")            
        err = mx.sysfile_write(self.fh, &count, <char*>bufptr)
        if err != mx.MAX_ERR_NONE:
            raise IOError("could not write contents file handler")
        mx.sysmem_freeptr(bufptr)
        mx.post("wrote %d bytes to file handler", count)

    def sysfile_seteof(self, int nbytes):
        """Set the size of the file handle in bytes."""
        cdef mx.t_max_err err = mx.sysfile_seteof(self.fh, <mx.t_ptr_size>nbytes)
        if err != mx.MAX_ERR_NONE:
            raise ValueError("could not set the size of the file handle")

    def sysfile_geteof(self) -> int:
        """Get the size of a file handle."""
        cdef mx.t_ptr_size nbytes = 0
        cdef mx.t_max_err err = mx.sysfile_geteof(self.fh, &nbytes)
        if err != mx.MAX_ERR_NONE:
            raise ValueError("could not get the size of the file handle")
        return <int>nbytes

    def sysfile_getpos(self) -> int:
        """Get the current file position of a file handle."""
        cdef mx.t_ptr_size filepos = 0
        cdef mx.t_max_err err = mx.sysfile_getpos(self.fh, &filepos)
        if err != mx.MAX_ERR_NONE:
            raise ValueError("could not get the position of the file handle")
        return <int>filepos

    cdef mx.t_max_err sysfile_spoolcopy(self, mx.t_filehandle src, mx.t_filehandle dst, mx.t_ptr_size size):
        """Copy the contents of one file handle to another file handle.

        @param  src   The file handle from which to copy.
        @param  dst   The file handle to which the copy is written.
        @param  size  The number of bytes to copy.  If 0 the size of src will be used.
        @return     An error code.
        """
        return mx.sysfile_spoolcopy(src, dst, size)

    def sysfile_readtextfile(self, int maxlen = 0, int flags = 0) -> str:
        """Read a text file from disk.

        This function reads up to the maximum number of bytes given by 
        maxlen from file handle at current file position into the htext 
        handle, performing linebreak translation if set in flags.
        """
        cdef int size = self.sysfile_geteof()
        cdef mx.t_handle htext = <mx.t_handle>mx.sysmem_newhandleclear(<mx.t_ptr_size>size)
        cdef mx.t_max_err err = mx.MAX_ERR_NONE
        err = mx.sysfile_readtextfile(self.fh, 
            htext, <mx.t_ptr_size>maxlen, mx.TEXT_LB_NATIVE)
        if err != mx.MAX_ERR_NONE:
            raise ValueError("could not read text from the file handle")
        lines = []
        for i in range(size):
            line = htext[i].decode()
            lines.append(line)
        mx.sysmem_freehandle(htext)
        return "".join(lines)

    def sysfile_writetextfile(self, str text):
        """Write a text file to disk.

        This function writes a text handle to a text file performing linebreak 
        translation if set in flags.

        see: https://cycling74.com/forums/problem-with-sysfile_writetextfile
        """
        cdef char* buf = <bytes>text
        cdef mx.t_handle htext = mx.sysmem_newhandle(0)
        cdef mx.t_max_err err = mx.MAX_ERR_NONE

        mx.sysmem_ptrandhand(buf, htext, strlen(buf))
        err = mx.sysfile_writetextfile(self.fh, htext, mx.TEXT_LB_NATIVE)
        if err != mx.MAX_ERR_NONE:
            raise IOError("could not write text to handler")
        mx.sysfile_close(self.fh)
        mx.sysmem_freehandle(htext)

    cdef mx.t_max_err sysfile_openhandle(self, char **h, mx.t_sysfile_flags flags, mx.t_filehandle *fh):
        """Create a #t_filehandle from a pre-existing handle.

        @param    h   A handle for some data, data is *not* copied and *not* freed on file close. 
        @param    flags Pass 0 (additional flags are private).
        @param    fh    The address of a #t_filehandle which will be allocated.
        @return       An error code.
        """
        return mx.sysfile_openhandle(h, flags, fh)

    cdef mx.t_max_err sysfile_openptrsize(self, char *p, mx.t_ptr_size length, mx.t_sysfile_flags flags, mx.t_filehandle *fh):
        """Create a #t_filehandle from a pre-existing pointer.

        @param    p   A pointer to some data. Data is *not* copied and *not* freed on file close.
        @param    length  The size of p.
        @param    flags Pass 0 (additional flags are private).
        @param    fh    The address of a #t_filehandle which will be allocated.
        @return       An error code.
        """
        return mx.sysfile_openptrsize(p, length, flags, fh)
