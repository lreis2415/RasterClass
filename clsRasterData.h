/*!
 * \ingroup data
 * \brief Define Raster class to handle raster data
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0 is supported)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 * \revised Dec. 2016 Separated from SEIMS to a common library for widely use.
 * 
 */
#ifndef CLS_RASTER_DATA
#define CLS_RASTER_DATA
/// include base headers
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
/// include commonlibs/UtilsClass, required
#include "util.h"
#include "utils.h"
#include "ModelException.h"
/// include GDAL, required
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"
/// include MongoDB, optional
#ifdef USE_MONGODB

#include "mongoc.h"
#include "MongoUtil.h"

#endif


using namespace std;

/*!
 Define Raster related constant strings used for raster headers
 */
#define HEADER_RS_NODATA        "NODATA_VALUE"
#define HEADER_RS_XLL           "XLLCENTER"  /// or XLLCORNER
#define HEADER_RS_YLL           "YLLCENTER"  /// or YLLCORNER
#define HEADER_RS_NROWS         "NROWS"
#define HEADER_RS_NCOLS         "NCOLS"
#define HEADER_RS_CELLSIZE      "CELLSIZE"
#define HEADER_RS_LAYERS        "LAYERS"
#define HEADER_RS_SRS           "SRS"

/*!
 * Files or database constant strings
 */
#define ASCIIExtension          ".asc"
#define GTiffExtension          ".tif"

/*!
 * \class clsRasterData
 *
 * \brief Raster data (1D and 2D) I/O class
 * Support I/O between TIFF, ASCII file or/and MongoBD database.
 */
template<typename T>
class clsRasterData {
public:
    /*!
     * \brief Constructor an empty clsRasterData instance
     * By default, 1D raster data
     * Set \a m_rasterPositionData, \a m_rasterData, \a m_mask to \a NULL
     */
    clsRasterData(void);

    /*!
     * \brief Constructor of clsRasterData instance from TIFF, ASCII, or other GDAL supported raster file
     * By default, 1D raster data
     * \sa ReadASCFile() ReadFromGDAL()
     * \param[in] filename Full path of the raster file
     * \param[in] exludeNodata Excluding NODATA or not. The default is true.
     * \param[in] mask \a clsRasterData<int> Mask layer
     *
     */
    clsRasterData(string filename, bool exludeNodata = true, clsRasterData<int> *mask = NULL);

    /// Initialization function based on MongoDB
//#ifdef USE_MONGODB
//
//    /*!
//     * \brief Constructor of clsRasterData instance from MongoDB
//     * By default, 1D raster data
//     * \sa ReadFromMongoDB()
//     *
//     * \param[in] gfs \a mongoc_gridfs_t
//     * \param[in] remoteFilename \a char*
//     * \param[in] getAllValues Store all matrix values, or excluding NODATA. The default is false.
//     * \param[in] templateRaster \a clsRasterData, NULL as default
//     */
//    clsRasterData<T>(mongoc_gridfs_t *gfs, const char *remoteFilename, clsRasterData<T> *templateRaster = NULL);
//
//#endif

    //! Destructor, release \a m_rasterPositionData, \a m_rasterData and \a m_mask if existed.
    ~clsRasterData(void);

    /************* Read functions ***************/

    /*!
     * \brief Read raster data from ASC file, using mask
     * Be aware, this mask should overlay with the raster, otherwise, exception will be thrown.
     * \param[in] filename \a string
     * \param[in] exludeNodata Excluding NODATA or not. The default is true.
     * \param[in] mask \a clsRasterData
     */
    void ReadASCFile(string filename, bool exludeNodata = true, clsRasterData<int> *mask = NULL);

    /************* Write functions ***************/

    /*!
     * \brief Write raster to ASCII raster file, if 2D raster, output name will be filename_LyrNum
     * \param filename
     */
    void outputASCFile(string &filename);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(map<string, double> header, int nRows, float **position, T *value, string filename);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] header header information
     * \param[in] nRows \a int, valid cell number
     * \param[in] position \a float**, position index
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path, take the CoreName as prefix
     */
    static void outputASCFile(map<string, double> header, int nRows, float **position, T **value, string filename);

    /*!
     * \brief Write raster data into ASC file
     *
     * \param[in] templateRasterData
     * \param[in] value \a T*, Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(clsRasterData *templateRasterData, T *value, string filename);

    /*!
     * \brief Write 2D raster data into ASC file
     *
     * \param[in] templateRasterData \sa  clsRasterData
     * \param[in] value \a T**, 2D Raster data
     * \param[in] filename \a string, output ASC file path
     */
    static void outputASCFile(clsRasterData *templateRasterData, T **value, string filename);


    //! Excluding NoDATA or not
    bool excludingNodata() { return  m_excludeNODATA; }

    //! Get the average of raster data
    float getAverage();

    //! Get the average of the given layer of raster data
    float getAverage(int lyr);

    //! Get column number of raster data
    int getCols() { return (int) m_headers[HEADER_RS_NCOLS]; }

    //! Get row number of raster data
    int getRows() { return (int) m_headers[HEADER_RS_NROWS]; }

    //! Get cell size of raster data
    float getCellWidth() { return (float) m_headers[HEADER_RS_CELLSIZE]; }

    //! Get cell numbers ignore NoData
    int getCellNumber() { return m_nCells; }

    int getLayers() { return m_nLyrs; }

    //! Get NoDATA value of raster data
    T getNoDataValue() { return (T) m_headers[HEADER_RS_NODATA]; }

    //! Get position index in 1D raster data for specific row and column, return -1 is error occurs.
    int getPosition(int row, int col);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(float x, float y);

    //! Get position index in 1D raster data for given coordinate (x,y)
    int getPosition(double x, double y);

    //! Get raster data, include valid cell number and data
    void getRasterData(int *, T **);

    //! Get 2D raster data, include valid cell number of each layer, layer number, and data
    void get2DRasterData(int *, int *, T ***);

    //! Get raster header information
    map<string, double> *getRasterHeader(void);

    //! Get non-NODATA position index data, include cell number and (row, col)
    void getRasterPositionData(int *, T ***);

    //! Get pointer of raster data
    float *getRasterDataPointer() { return m_rasterData; }

    //! Get pointer of 2D raster data
    float **get2DRasterDataPointer() { return m_raster2DData; }

    //! Get the spatial reference
    const char *getSRS() { return m_srs.c_str(); }

    //! Get the spatial reference string
    string getSRSString() { return m_srs; }

    //! Get raster data at the valid cell index
    T getValue(int validCellIndex);

    //! Get raster data at the valid cell index (both for 1D and 2D raster), return a float array with length as nLyrs
    T *getValue(int validCellIndex, int *nLyrs);

    //! Get raster data at the row and col
    T getValue(int row, int col);

    //! Get raster data (both for 1D and 2D raster) at the row and col, return a float array with length as nLyrs
    T *getValue(int row, int col, int *nLyrs);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col);

    //! Get raster data value at row and column of \a templateRasterData and \a rasterData
    T *getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col, int *nLyrs);

    //! Get X coordinate of left lower corner of raster data
    double getXllCenter() { return m_headers[HEADER_RS_XLL]; }

    //! Get Y coordinate of left lower corner of raster data
    double getYllCenter() { return m_headers[HEADER_RS_YLL]; }

    //! Is 2D raster data?
    bool is2DRaster() { return m_is2DRaster; }


//    //! Write raster to GTIFF Grid file, if 2D raster, output name will be filename_LyrNum
//    void outputGTiff(string filename);
//
//    /*!
//     * \brief Write raster data into GTIFF file
//     *
//     * \param[in] templateRasterData \sa  clsRasterData
//     * \param[in] value \a T*, Raster data
//     * \param[in] rasterName \a string, output GTIFF file path
//     */
//    static void outputGTiff(clsRasterData *templateRasterData, T *value, string rasterName);
//
//    /*!
//     * \brief Write raster data into GTIFF file
//     *
//     * \param[in] header header information
//     * \param[in] srs spatial reference string
//     * \param[in] nValidCells \a int, valid cell number
//     * \param[in] position \a float**, position index
//     * \param[in] value \a T*, Raster data
//     * \param[in] filename \a string, output GTIFF file path
//     */
//    static void outputGTiff(map<string, double> header, string &srs, int nValidCells, float **position, T *value,
//                            string filename);
//
//    /*!
//     * \brief Write 2D raster data into GTIFF file
//     *
//     * \param[in] templateRasterData
//     * \param[in] value \a T**, Raster data
//     * \param[in] rasterName \a string, output GTIFF file path
//     */
//    static void outputGTiff(clsRasterData *templateRasterData, T **value, string rasterName);
//
//    /*!
//     * \brief Write 2D raster data into GTIFF file
//     *
//     * \param[in] header header information
//     * \param[in] srs spatial reference string
//     * \param[in] nValidCells \a int, valid cell number
//     * \param[in] position \a float**, position index
//     * \param[in] value \a T**, Raster data
//     * \param[in] filename \a string, output GTIFF file path
//     */
//    static void outputGTiff(map<string, double> header, string &srs, int nValidCells, float **position, T **value,
//                            string filename);
//
//    /*
//     * \brief Write 2D-array raster data into GTIFF file
//     *        Used when valid position data is not available.
//     */
//    static void outputGTiff(map<string, double> header, string &srs, T *value, string &filename);
//
//    /// Output functions based on MongoDB
//#ifdef USE_MONGODB
//
//    //! Write raster data to MongoDB, if 2D raster, output name will be filename_LyrNum
//    void outputToMongoDB(string remoteFilename, mongoc_gridfs_t *gfs);
//
//    /*!
//     * \brief Write raster data into MongoDB
//     *
//     * \param[in] header header information
//     * \param[in] srs spatial reference string
//     * \param[in] nValidCells \a int, valid cell number
//     * \param[in] position \a float**, position index
//     * \param[in] value \a T*, Raster data
//     * \param[in] filename \a string, output file name
//     * \param[in] gfs \a mongoc_gridfs_t
//     */
//    static void outputToMongoDB(map<string, double> header, string &srs, int nValid, float **position, T *value,
//                                string remoteFilename, mongoc_gridfs_t *gfs);
//
//    /*!
//     * \brief Write raster data into MongoDB
//     *
//     * \param[in] templateRasterData \a clsRasterData
//     * \param[in] value \a T*, Raster data
//     * \param[in] filename \a string, output file name
//     * \param[in] gfs \a mongoc_gridfs_t
//     */
//    static void outputToMongoDB(clsRasterData *templateRasterData, T *value, string filename, mongoc_gridfs_t *gfs);
//
//    /*!
//     * \brief Write 2D raster data into MongoDB
//     *
//     * \param[in] header header information
//     * \param[in] srs spatial reference string
//     * \param[in] nValidCells \a int, valid cell number
//     * \param[in] position \a float**, position index
//     * \param[in] value \a T** 2D RasterData
//     * \param[in] lyrs \a layer number
//     * \param[in] filename \a string, output file name
//     * \param[in] gfs \a mongoc_gridfs_t
//     */
//    static void outputToMongoDB(map<string, double> header, string &srs, int nValid, float **position, T **value,
//                                int lyrs, string remoteFilename, mongoc_gridfs_t *gfs);
//
//    /*!
//     * \brief Write 2D raster data into MongoDB
//     *
//     * \param[in] templateRasterData \a clsRasterData
//     * \param[in] value \a T**, Raster data with multi-layers, e.g., soil properties
//     * \param[in] lyrs \a layer number
//     * \param[in] filename \a string, output file name
//     * \param[in] gfs \a mongoc_gridfs_t
//     */
//    static void
//    outputToMongoDB(clsRasterData *templateRasterData, T **value, int lyrs, string filename, mongoc_gridfs_t *gfs);
//
//#endif
//
//    /*!
//     * \brief Write weight file according the weight value
//     * \param[in] templateRasterData \a clsRasterData
//     * \param[in] nCols \a int i.e., HydroClimate site number
//     * \param[in] weight \a float
//     * \param[in] filename \a char*, weight file name
//     */
//    static void outputWeightFile(clsRasterData *templateRasterData, int nCols, float weight, string filename);
//
//
//    /*!
//     * \brief Read raster data using GDAL
//     *
//     * \param[in] filename \a string
//     */
//    void ReadFromGDAL(string filename);
//
//    /*!
//     * \brief Read raster data according the the given mask file using GDAL
//     * Be aware, this mask should have the same extent with the raster
//     * i.e., NROWS and NCOLS are the same!
//     * \param[in] filename \a string
//     * \param[in] mask \a clsRasterData
//     */
//    void ReadFromGDAL(string filename, clsRasterData *mask);
//
//    /// Read functions based on MongoDB
//#ifdef USE_MONGODB
//
//    /*!
//     * \brief Read raster data from MongoDB
//     * \param[in] gfs \a mongoc_gridfs_t
//     * \param[in] remoteFilename \a char*, raster file name
//     */
//    int ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename);
//
//    /*!
//     * \brief Read raster data according the the given mask file from MongoDB
//     * Be aware, this mask should have the same extent with the raster
//     * i.e., NROWS and NCOLS are the same!
//     * \param[in] gfs \a mongoc_gridfs_t
//     * \param[in] remoteFilename \a char*, raster file name
//     * \param[in] mask \a clsRasterData
//     */
//    int ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename, clsRasterData *mask);
//
//#endif

    /*!
     * \brief Get cell number
     * \sa getCellNumber()
     */
    int Size() { return m_nCells; }

    /************* Utility functions ***************/

    /*!
     * \brief Calculate XY coordinates by given row and col number
     * \param row
     * \param col
     * \return double[2]
     */
    double* getCoordinateByRowCol(int row, int col);

    /*!
     * \brief Calculate position by given coordinate
     * \param x
     * \param y
     * \return
     */
    int* getPositionByCoordinate(double x, double y);

    /*!
     * \brief Copy header information to current Raster data
     * \param[in] refers
     */
    void copyHeader(map<string, double> *refers);
private:
    ///< noDataValue
    T m_noDataValue;
    ///< raster file name
    string m_fileName;
    ///< get all matrix values, or excluding NODATA. The default is true, means excluding NODATA.
    bool m_excludeNODATA = true;
    ///< raster data (1D array)
    T *m_rasterData;
    ///< cell index (row, col) in m_rasterData (2D array)
    int **m_rasterPositionData;
    ///< cell number of raster data (exclude NODATA_VALUE)
    int m_nCells;
    ///< Header information, using double in case of truncation of coordinate value
    map<string, double> m_headers;
    ///< mask clsRasterData instance
    clsRasterData<int> *m_mask;

    //! raster data (2D array)
    T **m_raster2DData;
    //! Flag to identify 1D or 2D raster
    bool m_is2DRaster;
    //! Layer number of the 2D raster
    int m_nLyrs;
    //! OGRSpatialReference
    string m_srs;

//private:
//	void DeleteExistingData(void);
//public:
//	bool IsNull(int i, int j);
//	bool IsEmpty(void);
};

#endif