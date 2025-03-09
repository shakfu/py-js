
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

# alternative constructor (WIP)
cdef class MaxObject:
    def __init__(self, str classname, str name = "", *args, str namespace = "box"):
        cdef Atom atom = Atom(*args) if args else None
        self.name = name
        self.classname = classname
        self.namespace = namespace
        if name and not classname and namespace:
            # assume there's a registered object with the name, get it
            self.ptr = <mx.t_object*>mx.object_findregistered(
                str_to_sym(namespace) , str_to_sym(name))
            if self.ptr is NULL:
                raise ValueError("could not retrieve the coll object with name '{name}'")
            # now try to get its classname
            self.classname = sym_to_str(mx.object_classname(<mx.t_object*>self.ptr))
            # mx.object_classname_compare(self.ptr, gensym(classname))
        elif name and classname and namespace:
            self.ptr = <mx.t_object*>mx.object_new_typed(
                str_to_sym(namespace), str_to_sym(classname), atom.size, atom.ptr)
        elif classname and name:
            pass
        else:
            raise TypeError("must use name or classname")




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


# Matrix 

# Matrix.unused -----


    def set_plane2d(self, object value, int x, int y, int plane=0):
        """Set a 2-dimensional cell to specified values

        The word `setcell2d`, followed by a pair of numbers specifying `x` and
        `y` coordinates and a list of values, is similar to the `setcell`
        message but without the need to use a `val` token to separate the
        coordinates from the value since the dimension count (2) is fixed.

        Note that the order is slightly different in the python version of 
        of this method, so for the max message `(setplane2d 3 2 1 4)`, the
        equivalent in python is (with value being first):

        >>> matrix.set_plate2d(4, x=3, y=2, plane=1)
        """
        cdef Atom atom = Atom.from_seq((x, y, plane, value))
        jt.jit_object_method(<jt.t_object*>self.ptr, mx.gensym("setplane2d"),
            atom.size, atom.ptr)

    def set_char_data(self, list[int] data):
        """set data to whole matrix"""
        cdef int k = 0
        cdef int i, j, p
        cdef char *m_ptr = NULL

        for i in range(self.height):
            m_ptr = self.data + i * self.info.dimstride[1]
            for j in range(self.width):
                for p in range(self.planecount):
                    # (m_ptr+p)[0] = 2 # doesn't work!
                    m_ptr[0] = 2
                    m_ptr += 1


# Matrix.WIP

    cdef void* cell_ptr_1d(self, int x):
        """Retrieves pointer to directly access matrix cells if it is 1D"""
        return <void*>(<jt.uchar*>self.data + self.info.dimstride[0] * x)

    cdef void* cell_ptr_2d(self, int x, int y):
        return <void*>(<jt.uchar*>self.data + self.info.dimstride[0] * x
                                            + self.info.dimstride[1] * y)

    cdef void* cell_ptr_3d(self, int x, int y, int z):
        return <void*>(<jt.uchar*>self.data + self.info.dimstride[0] * x
                                            + self.info.dimstride[1] * y
                                            + self.info.dimstride[2] * z)


    def set_cell2d_char(self, int value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data as unsigned char using a contiguous array."""
        # assert 0 <= plane < self.planecount, "plane out of range"
        cdef char* p = <char*>self.cell_ptr_2d(x, y)
        cdef long savelock = <long>self.lock()
        p[plane] = <jt.uchar>clamp(value, 0, 255)
        self.unlock(savelock)

    def set_cell2d_char2(self, int value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data as unsigned char using a contiguous array."""
        # assert 0 <= plane < self.planecount, "plane out of range"
        cdef char* p = <char*>self.cell_ptr_2d(x, y)
        cdef long savelock = <long>self.lock()
        p[plane] = <jt.uchar>clamp(value, 0, 255)
        p[plane+1] = <jt.uchar>clamp(value+1, 0, 255)
        self.unlock(savelock)

    def set_cell2d_long(self, long value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data as long using a contiguous array."""
        assert 0 <= plane < self.planecount, "plane out of range"
        cdef long* p = <long*>self.cell_ptr_2d(x, y)
        cdef long savelock = <long>self.lock()
        p[plane] = <long>value
        self.unlock(savelock)

    def set_cell2d_float(self, float value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data as float using a contiguous array."""
        assert 0 <= plane < self.planecount, "plane out of range"
        cdef float* p = <float*>self.cell_ptr_2d(x, y)
        cdef long savelock = <long>self.lock()
        p[plane] = <float>value
        self.unlock(savelock)

    def set_cell2d_double(self, double value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data as double using a contiguous array."""
        assert 0 <= plane < self.planecount, "plane out of range"
        cdef double* p = <double*>self.cell_ptr_2d(x, y)
        cdef long savelock = <long>self.lock()
        p[plane] = <double>value
        self.unlock(savelock)

    def set_cell2d(self, object value, int x = 0, int y = 0, int plane = 0):
        """sets the matrix's data using a contiguous array."""
        if self.type == "char":
            self.set_cell2d_char(value, x, y, plane)
        elif self.type == "long":
            self.set_cell2d_long(value, x, y, plane)
        elif self.type == "float32":
            self.set_cell2d_float(value, x, y, plane)
        elif self.type == "float64":
            self.set_cell2d_double(value, x, y, plane)
        else:
            raise TypeError("could not process this type")


    def set_char_data(self, data: list[int], int x = 0, int y = 0):
        """sets the matrix's data as unsigned char using a contiguous array."""
        cdef jt.uchar entry = 0
        cdef char* p = NULL
        assert len(data) <= self.matrix_len, "incoming data not <= equal matrix_len"
        cdef long savelock = <long>self.lock()
        p = <char*>self.cell_ptr_2d(x, y)
        for i in range(self.planecount):
            p[i] = <jt.uchar>clamp(data[i], 0, 255)
        self.unlock(savelock)

    def set_long_data(self, data: list[int], int x = 0, int y = 0):
        """sets the matrix's data as long using a contiguous array."""
        cdef long entry = 0
        assert len(data) <= self.matrix_len, "incoming data not <= equal matrix_len"
        cdef long savelock = <long>self.lock()
        cdef long* p = <long*>self.cell_ptr_2d(x, y)
        for i in range(len(data)):
            entry = <long>data[i]
            p[i] = entry
        self.unlock(savelock)

    def set_float_data(self, data: list[float], int x = 0, int y = 0):
        """sets the matrix's data as float using a contiguous array."""
        cdef float entry = 0
        assert len(data) <= self.matrix_len, "incoming data not <= equal matrix_len"
        cdef long savelock = <long>self.lock()
        cdef float* p = <float*>self.cell_ptr_2d(x, y)
        for i in range(len(data)):
            entry = <float>data[i]
            p[i] = entry
        self.unlock(savelock)

    def set_double_data(self, data: list[float], int x = 0, int y = 0):
        """sets the matrix's data as double using a contiguous array."""
        cdef double entry = 0
        assert len(data) <= self.matrix_len, "incoming data not <= equal matrix_len"
        cdef long savelock = <long>self.lock()
        cdef double* p = <double*>self.cell_ptr_2d(x, y)
        for i in range(len(data)):
            entry = <double>data[i]
            p[i] = entry
        self.unlock(savelock)

    def set_data(self, data: list[object], int x = 0, int y = 0):
        """sets the matrix's data using a contiguous array."""
        if self.type == "char":
            self.set_char_data(data, x, y)
        elif self.type == "long":
            self.set_long_data(data, x, y)
        elif self.type == "float32":
            self.set_float_data(data, x, y)
        elif self.type == "float64":
            self.set_double_data(data, x, y)
        else:
            raise TypeError("could not process this type")

    def set_char_data(self, list[int] data):
        """set data to whole matrix"""
        cdef int j = 0
        cdef int x = 0
        cdef char* p = NULL
        for plane in range(self.planecount):
            self.data += plane
            for i in range(len(data)):
                x = (j // self.info.dim[0]) * self.info.dimstride[1] + (j % self.info.dim[0]) * self.info.dimstride[0]
                post(f"x = {x}")
                p = self.data + (j // self.info.dim[0]) * self.info.dimstride[1] + (j % self.info.dim[0]) * self.info.dimstride[0]
                (<jt.uchar*>p)[0] = <jt.uchar>clamp(data[i], 0, 255)
                j += 1
                # post(f"(p, j, i) = ({plane}, {j}, {i})")
            j = 0



    def set_data(self, data: list[int], int x = 0, int y = 0):
        """sets the matrix's data using a contiguous array."""
        assert len(data) <= self.matrix_len, "incoming data not <= equal matrix_len"
        cdef long savelock = <long>self.lock()
        cdef char* p = <char*>self.cell_ptr_2d(x, y)
        for i in range(len(data)):
            p[i] = <char>data[i]
        self.unlock(savelock)

    def set_data(self, char[:,:,:] matrix):
        """Set matrix from a memoryview
        
        >>> np.arange(200).reshape((2,20,5))

        """
        cdef long planes = <Py_ssize_t>matrix.shape[0]
        cdef long rows = <Py_ssize_t>matrix.shape[1]
        cdef long cols = <Py_ssize_t>matrix.shape[2]

        assert cols == self.dim[0]
        assert rows == self.dim[1]
        assert planes == self.planecount
        assert cols * rows * planes == self.matrix_len
        # cdef int i = 0
        # self.lock()
        # ...
        # self.unlock()
    



