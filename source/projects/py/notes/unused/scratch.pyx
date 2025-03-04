
# Path wrongly comnines all Path functyions with sysfile funcs.
# How to separate them to make sense??

"""
% rg -c 't_filehandle'
ext_path.h:5
ext_sndfile.h:1
ext_xmltree.h:1
ext_sysfile.h:33

% rg -c 't_handle'
max_types.h:1
ext_obex_util.h:1
ext_proto.h:1
ext_sysfile.h:2
ext_sysmem.h:12
c74_linker_flags.txt:1
"""


in min-api:


ctypedef enum DirType:
    UNDEFINED   = 0
    APPLICATION = 1
    DESKTOP     = 2
    PREFERENCES = 3
    TEMP        = 4

ctypedef enum FileType:
    ANY         = 0
    FOLDER      = 1
    EXTERNAL    = 2
    PATCHER     = 3
    AUDIO       = 4


cdef class Path:
    """A wrapper for Max friendly paths"""

    cdef public str filename    # name used to search for file
    cdef public str pathname    # absolute path after file is found
    cdef short path_id          # max short code for parent folder
    cdef mx.t_fourcc type       # fourchar code of file
    cdef bint is_directory      # true if path is a directory
    cdef mx.t_filehandle fh     # file handle

    def __cinit__(self):
        self.filename = ""
        self.pathname = ""
        self.path_id = 0
        self.type = <mx.t_fourcc>fourchar_to_int('TEXT')
        self.is_directory = False
        self.fh = NULL

    # def __init__(self, str filename):
    #     self.filename = filename
    #     self.pathname = self.locate(filename)
    #     self._get_info() # populate info attribute

    def __init__(self, str name, FileType type = FileType.ANY, bint create = False):
        cdef char pathname[2048]     # absolute path result of search
        cdef char filename[512]
        cdef mx.t_fourcc outtype = 0
        cdef short err = 0
        cdef char* foldername = NULL
        cdef short parent_folder_path = 0



        # self.filename = filename

        if (type == FileType.FOLDER):
            self.is_directory = True

        # return self.to_absolute_path(filename, self.path_id)

        err = mx.locatefile_extended(name.encode(), &self.path_id, &outtype, &self.type, 1)
        if err: # there is an error
            if create:
                if type == FileType.FOLDER:
                    mx.path_nameconform(name.encode(), pathname, mx.PATH_STYLE_MAX, mx.PATH_TYPE_ABSOLUTE)
                    foldername = strrchr(pathname, <int>ord('/'))
                    parent_folder_path = 0

                    if (foldername):
                        foldername[0] = 0
                        foldername += 1

                        err = mx.path_frompathname(pathname, &parent_folder_path, filename)
                        if not err:
                            err = mx.path_createfolder(parent_folder_path, foldername, &self.path_id)
                        if err:
                            raise IOError("error trying to create folder")
                    else:
                        raise IOError("no folder name provided")
            else:
                raise IOError("file not found")
        else:
            if self.type == fourchar_to_int('fold'):
                self.is_directory = True

        if self.is_directory:
            err = mx.path_getpath(self.path_id, filename, &self.path_id)
            if err:
                raise IOError("folder not found")



enum system
{
    undefined = 0,
    application,
    desktop,
    preferences,
    temp
};

enum filetype
{
    any = 0,
    folder,
    external,
    patcher,
    audio
};

ctypedef t_ptr_uint filedate 


# min-api's path class does not combine path and sysfile

cdef class path:

        cdef short m_path{}
        cdef char m_filename[max::MAX_PATH_CHARS]{}
        cdef max::t_fourcc m_type{}
        cdef bool m_directory{}

    def __init__(self)
        """uninitialized path"""
    def __init__(self, system initial)
        """path initialized to a system directory"""
    def __init__(self, short path_id)
        """path initialized to a user-supplied path id (discouraged, but might be provided by legacy Max API)"""
    def __init__(self, str name, filetype type = any, bool create = False)
        """path initialized by name"""
    def __init__(self, Atom name, filetype type = any)

    def __str__(self)
    def __bool__(self): return self.path_id != 0
    def __iter__(self)
    
    def typelist(self, filetype type) -> list[t_fourcc]
    def date_modified(self) -> filedate
    def enumerate(self, filetype type, func[str] callback)
    def name(self) -> str
    def copy(self, Path dest_folder, str dest_name)


# if combined there is too much redundancy, so how about this:

cdef class Path:

    cdef public str filename    # name used to search for file
    cdef public bool is_dir     # true if path is a directory
    cdef short path_id          # max short code for parent folder
    cdef mx.t_fourcc type       # unsigned int created from 4 chars

    def __cinit__(self)
    def __init__(self, str filename)
    def __str__(self)
    def __repr__(self)
    def __enter__(self) -> Path
    def __exit__(self)
    def __iter__(self)

    def set_default_path(self, short path_id, bint recursive = False)
    def to_absolute_path(self, str filename, short path_id) -> str
    def to_pathname(self, str filename, int path_id)
    def from_pathname(self, str pathname) -> Path
    def create_file(self, str filename, short path_id) -> int
    def locate(self, str filename, str code = 'TEXT') -> str
    def rename(self, str new_name)
    def delete(self)
    def open_sysfile(self, str filename, short path_id, str perm = 'w')
    def create_sysfile(self, str filename, short path_id, str fourchar_type)


class SysFile:

    cdef public str pathname    # absolute path
    cdef public str filename    # name used to search for file
    cdef short path_id          # max short code for parent folder
    cdef mx.t_fourcc type       # unsigned int created from 4 chars

    cdef mx.t_filehandle fh     # file handle
    cdef int size               # size of file handle file

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


