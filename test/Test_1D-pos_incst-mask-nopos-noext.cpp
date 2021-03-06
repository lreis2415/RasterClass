/*!
 * @brief Test description:
 *                      CalcPositions UseMaskExtent ExtentConsistent  SingleLayer
 *        Raster data:     YES             NO            NO               YES
 *        Mask data  :      NO             --            NO               YES
 *
 *        TEST CASE NAME (or TEST SUITE): 
 *            clsRasterDataTestPosIncstMaskNoPosNoExt
 *
 *        Since we mainly support ASC and GDAL(e.g., TIFF),
 *        value-parameterized tests of Google Test will be used.
 * @cite https://github.com/google/googletest/blob/master/googletest/samples/sample7_unittest.cc
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/05/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

using namespace std;

namespace {
#if GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::Values;

string apppath = GetAppPath();
string corename = "dem_2";
string maskcorename = "mask1";
string asc_file = apppath + "../data/" + corename + ".asc";
string tif_file = apppath + "../data/" + corename + ".tif";
string mask_asc_file = apppath + "../data/" + maskcorename + ".asc";
string mask_tif_file = apppath + "../data/" + maskcorename + ".tif";

struct inputRasterFiles {
public:
    inputRasterFiles(const string &rsf, const string &maskf) {
        raster_name = rsf.c_str();
        mask_name = maskf.c_str();
    };
    const char *raster_name;
    const char *mask_name;
};

//Inside the test body, fixture constructor, SetUp(), and TearDown() you
//can refer to the test parameter by GetParam().  In this case, the test
//parameter is a factory function which we call in fixture's SetUp() to
//create and store an instance of clsRasterData<float>.
class clsRasterDataTestPosIncstMaskNoPosNoExt : public TestWithParam<inputRasterFiles *> {
public:
    clsRasterDataTestPosIncstMaskNoPosNoExt() : rs(nullptr), maskrs(nullptr) {}
    virtual ~clsRasterDataTestPosIncstMaskNoPosNoExt() { delete rs; }
    virtual void SetUp() {
        // Read mask data with default parameters, i.e., calculate valid positions.
        maskrs = clsRasterData<int>::Init(GetParam()->mask_name, false);
        ASSERT_NE(nullptr, maskrs);
        // Read raster data with the masked data
        rs = clsRasterData<float, int>::Init(GetParam()->raster_name, true, maskrs, false);
        ASSERT_NE(nullptr, rs);
    }
    virtual void TearDown() {
        delete rs;
        delete maskrs;
        rs = nullptr;
        maskrs = nullptr;
    }
protected:
    clsRasterData<float, int> *rs;
    clsRasterData<int> *maskrs;
};

// Since each TEST_P will invoke SetUp() and TearDown()
// once, we put all tests in once test case. by lj.
TEST_P(clsRasterDataTestPosIncstMaskNoPosNoExt, RasterIO) {
    /// 1. Test members after constructing.
    EXPECT_EQ(60, rs->getDataLength());  // m_nCells
    EXPECT_EQ(60, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ(corename, rs->getCoreName());  // m_coreFileName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_FALSE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_TRUE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(rs->validate_raster_data());

    EXPECT_NE(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_EQ(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    map<string, double> header_info = rs->getRasterHeader();
    EXPECT_FLOAT_EQ(header_info.at("LAYERS"), rs->getLayers());
    EXPECT_FLOAT_EQ(header_info.at("CELLSNUM"), rs->getCellNumber());

    EXPECT_EQ(8, rs->getRows());
    EXPECT_EQ(10, rs->getCols());
    EXPECT_FLOAT_EQ(19.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(25.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->getCellWidth());
    EXPECT_EQ(1, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap **/
    EXPECT_EQ(60, rs->getValidNumber());
    EXPECT_FLOAT_EQ(7.07f, rs->getMinimum());
    EXPECT_FLOAT_EQ(98.49f, rs->getMaximum());
    EXPECT_FLOAT_EQ(10.23766667f, rs->getAverage());
    EXPECT_FLOAT_EQ(11.52952953f, rs->getSTD());
    EXPECT_FLOAT_EQ(91.42f, rs->getRange());
    EXPECT_TRUE(rs->StatisticsCalculated());


    EXPECT_NE(nullptr, rs->getMask());  // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float *rs_data = nullptr;
    EXPECT_TRUE(rs->getRasterData(&ncells, &rs_data));  // m_rasterData
    EXPECT_EQ(60, ncells);
    EXPECT_NE(nullptr, rs_data);
    EXPECT_FLOAT_EQ(7.94f, rs_data[0]);
    EXPECT_FLOAT_EQ(9.85f, rs_data[59]);
    EXPECT_FLOAT_EQ(9.75f, rs_data[16]);

    float **rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_FALSE(rs->get2DRasterData(&ncells, &nlyrs, &rs_2ddata));  // m_raster2DData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(-1, nlyrs);
    EXPECT_EQ(nullptr, rs_2ddata);

    /** Get raster cell value by various way **/
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(7.94f, rs->getValueByIndex(0));
    EXPECT_FLOAT_EQ(9.85f, rs->getValueByIndex(59, 1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(60, 1));
    EXPECT_FLOAT_EQ(9.75f, rs->getValueByIndex(16));
    EXPECT_FLOAT_EQ(9.95f, rs->getValueByIndex(18));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 2));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(90, 2));

    int tmp_lyr;
    float *tmp_values;
    rs->getValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValueByIndex(1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.62f, tmp_values[0]);

    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(9, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 10));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.77f, rs->getValue(1, 4));
    EXPECT_FLOAT_EQ(8.77f, rs->getValue(1, 4, 1));

    rs->getValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.94f, tmp_values[0]);
    rs->getValue(1, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(1, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.54f, tmp_values[0]);

    // Get position
    EXPECT_EQ(8, rs->getPosition(22.05f, 37.95f));  // row 1, col 2
    EXPECT_EQ(8, rs->getPosition(23.95f, 36.05f));

    /** Set value **/

    // Set raster data value
    rs->setValue(1, 4, 0.877f);
    EXPECT_FLOAT_EQ(0.877f, rs->getValue(1, 4));
    rs->setValue(0, 2, 1.f);
    EXPECT_NE(1.f, rs->getValue(0, 2));

    // update statistics
    rs->updateStatistics();  // Should be manually invoked in your project!
    EXPECT_FLOAT_EQ(0.877f, rs->getMinimum());
    EXPECT_FLOAT_EQ(10.10611667f, rs->getAverage());
    EXPECT_FLOAT_EQ(11.59039314f, rs->getSTD());
    EXPECT_FLOAT_EQ(97.613f, rs->getRange());

    // Set core file name
    string newcorename = corename + "_1D-pos_incst-mask-nopos-noext";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string fakefullname = GetPathFromFullName(oldfullname) + "noExistDir" + SEP +
        "noOut" + "." + GetSuffix(oldfullname);
    EXPECT_FALSE(rs->outputToFile(fakefullname));
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
        newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->outputToFile(newfullname));
    EXPECT_TRUE(FileExists(newfullname));
}
INSTANTIATE_TEST_CASE_P(SingleLayer, clsRasterDataTestPosIncstMaskNoPosNoExt,
                        Values(new inputRasterFiles(asc_file, mask_asc_file),
                               new inputRasterFiles(tif_file, mask_tif_file)));
#else
TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif /* GTEST_HAS_PARAM_TEST */
} /* namespace */
