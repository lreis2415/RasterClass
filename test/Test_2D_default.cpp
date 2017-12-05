/*!
 * @brief Test clsRasterData to handle multiple layer data with all default parameters.
 *        i.e., Read multiple raster layer from files, calculate positions of valid cells,
 *              and no mask layer.
 *        Since the core code is irrelevant with the format of raster data, we take tiff
 *        as example here.
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/04/2017 lj Initial version.
 *
 */
#include "gtest/gtest.h"
#include "utilities.h"
#include "clsRasterData.h"

namespace {

TEST(clsRasterDataTest2DDefault, RasterIO) {
    /// 0. Read multiple raster data.
    string apppath = GetAppPath();
    vector<string> filenames;
    filenames.push_back(apppath + "../data/dem_1.tif");
    filenames.push_back(apppath + "../data/dem_2.tif");
    filenames.push_back(apppath + "../data/dem_3.tif");
    clsRasterData<float>* rs = new clsRasterData<float>(filenames);

    /// 1. Test members after constructing.
    EXPECT_EQ(545, rs->getDataLength());  // m_nCells
    EXPECT_EQ(545, rs->getCellNumber());  // m_nCells

    EXPECT_FLOAT_EQ(-9999.f, rs->getNoDataValue());  // m_noDataValue
    EXPECT_FLOAT_EQ(-9999.f, rs->getDefaultValue());  // m_defaultValue

    // m_filePathName depends on the path of build, so no need to test.
    EXPECT_EQ("dem", rs->getCoreName());  // m_coreFileName
    EXPECT_EQ("dem_%d", GetCoreFileName(rs->getFilePath())); // m_filePathName

    EXPECT_TRUE(rs->Initialized());  // m_initialized
    EXPECT_TRUE(rs->is2DRaster());  // m_is2DRaster
    EXPECT_TRUE(rs->PositionsCalculated());  // m_calcPositions
    EXPECT_TRUE(rs->PositionsAllocated());  // m_storePositions
    EXPECT_FALSE(rs->MaskExtented());  // m_useMaskExtent
    EXPECT_FALSE(rs->StatisticsCalculated());  // m_statisticsCalculated

    ASSERT_TRUE(rs->validate_raster_data());

    EXPECT_EQ(nullptr, rs->getRasterDataPointer());  // m_rasterData
    EXPECT_NE(nullptr, rs->get2DRasterDataPointer());  // m_raster2DData
    EXPECT_NE(nullptr, rs->getRasterPositionDataPointer());  // m_rasterPositionData

    /** Get metadata, m_headers **/
    EXPECT_EQ(20, rs->getRows());
    EXPECT_EQ(30, rs->getCols());
    EXPECT_FLOAT_EQ(1.f, rs->getXllCenter());
    EXPECT_FLOAT_EQ(1.f, rs->getYllCenter());
    EXPECT_FLOAT_EQ(2.f, rs->getCellWidth());
    EXPECT_EQ(3, rs->getLayers());
    EXPECT_STREQ("", rs->getSRS());
    EXPECT_EQ("", rs->getSRSString());

    /** Calc and get basic statistics, m_statsMap2D **/
    // layer 1
    EXPECT_EQ(545, rs->getValidNumber(1));
    EXPECT_FLOAT_EQ(7.07f, rs->getMinimum(1));
    EXPECT_FLOAT_EQ(10.f, rs->getMaximum(1));
    EXPECT_FLOAT_EQ(8.693963f, rs->getAverage(1));
    EXPECT_FLOAT_EQ(0.870768f, rs->getSTD(1));
    EXPECT_FLOAT_EQ(2.93f, rs->getRange(1));
    // layer 2
    EXPECT_EQ(541, rs->getValidNumber(2));
    EXPECT_FLOAT_EQ(2.75f, rs->getMinimum(2));
    EXPECT_FLOAT_EQ(98.49f, rs->getMaximum(2));
    EXPECT_FLOAT_EQ(9.20512f, rs->getAverage(2));
    EXPECT_FLOAT_EQ(5.612893f, rs->getSTD(2));
    EXPECT_FLOAT_EQ(95.74f, rs->getRange(2));
    // layer 3
    EXPECT_EQ(540, rs->getValidNumber(3));
    EXPECT_FLOAT_EQ(0.6f, rs->getMinimum(3));
    EXPECT_FLOAT_EQ(10.f, rs->getMaximum(3));
    EXPECT_FLOAT_EQ(8.502796f, rs->getAverage(3));
    EXPECT_FLOAT_EQ(1.382485f, rs->getSTD(3));
    EXPECT_FLOAT_EQ(9.4f, rs->getRange(3));

    EXPECT_TRUE(rs->StatisticsCalculated());

    EXPECT_EQ(nullptr, rs->getMask());  // m_mask

    /** Test getting raster data **/
    int ncells = 0;
    float* rs_data = nullptr;
    EXPECT_FALSE(rs->getRasterData(&ncells, &rs_data));  // m_rasterData
    EXPECT_EQ(-1, ncells);
    EXPECT_EQ(nullptr, rs_data);

    float** rs_2ddata = nullptr;
    int nlyrs = -1;
    EXPECT_TRUE(rs->get2DRasterData(&ncells, &nlyrs, &rs_2ddata));  // m_raster2DData
    EXPECT_EQ(545, ncells);
    EXPECT_EQ(3, nlyrs);
    EXPECT_NE(nullptr, rs_2ddata);
    // raster layer 1
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[0][0]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][0]);
    EXPECT_FLOAT_EQ(7.14f, rs_2ddata[4][0]);
    // raster layer 2
    EXPECT_FLOAT_EQ(9.9f, rs_2ddata[0][1]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][1]);
    EXPECT_FLOAT_EQ(27.14f, rs_2ddata[4][1]);
    // raster layer 3
    EXPECT_FLOAT_EQ(1.9f, rs_2ddata[0][2]);
    EXPECT_FLOAT_EQ(7.21f, rs_2ddata[544][2]);
    EXPECT_FLOAT_EQ(-9999.f, rs_2ddata[4][2]);

    /** Get raster cell value by various way **/
    // invalid inputs which return nodata
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(545));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(29, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(545, 4));
    // valid inputs
    EXPECT_FLOAT_EQ(9.9f, rs->getValueByIndex(0));  // layer 1 by default
    EXPECT_FLOAT_EQ(7.21f, rs->getValueByIndex(544, 1));
    EXPECT_FLOAT_EQ(7.14f, rs->getValueByIndex(4, 1));  // layer 1
    EXPECT_FLOAT_EQ(27.14f, rs->getValueByIndex(4, 2));  // layer 2
    EXPECT_FLOAT_EQ(-9999.f, rs->getValueByIndex(4, 3));  // layer 3

    int tmp_lyr;
    float *tmp_values;
    rs->getValueByIndex(-1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValueByIndex(4, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(7.14f, tmp_values[0]);
    EXPECT_FLOAT_EQ(27.14f, tmp_values[1]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[2]);

    // Get value by row, col, and layer (optional)
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(-1, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 30));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, -1));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 4));

    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4, 1));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4, 2));
    EXPECT_FLOAT_EQ(8.06f, rs->getValue(2, 4, 3));

    rs->getValue(-1, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, -1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(-1, tmp_lyr);
    EXPECT_EQ(nullptr, tmp_values);
    rs->getValue(0, 0, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[0]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[1]);
    EXPECT_FLOAT_EQ(-9999.f, tmp_values[2]);
    rs->getValue(0, 1, &tmp_lyr, &tmp_values);
    EXPECT_EQ(3, tmp_lyr);
    EXPECT_NE(nullptr, tmp_values);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[0]);
    EXPECT_FLOAT_EQ(9.9f, tmp_values[1]);
    EXPECT_FLOAT_EQ(1.9f, tmp_values[2]);

    // Get position
    EXPECT_EQ(29, rs->getPosition(4.05f, 37.95f));
    EXPECT_EQ(29, rs->getPosition(5.95f, 36.05f));

    /** Set value **/
    // Set core file name
    string corename = rs->getCoreName();
    string newcorename = corename + "_new_2Ddefault";
    rs->setCoreName(newcorename);
    EXPECT_EQ(newcorename, rs->getCoreName());

    // Set raster data value
    rs->setValue(2, 4, 18.06f);
    EXPECT_FLOAT_EQ(18.06f, rs->getValue(2, 4));
    rs->setValue(2, 4, 28.06f, 2);
    EXPECT_FLOAT_EQ(28.06f, rs->getValue(2, 4, 2));
    rs->setValue(2, 4, 38.06f, 3);
    EXPECT_FLOAT_EQ(38.06f, rs->getValue(2, 4, 3));
    rs->setValue(2, 4, 28.06f, 4);
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(2, 4, 4));
    rs->setValue(0, 0, 1.f);  // current version do not support setting value to NODATA location
    EXPECT_NE(1.f, rs->getValue(0, 0));
    EXPECT_FLOAT_EQ(-9999.f, rs->getValue(0, 0));

    /** Output to new file **/
    string oldfullname = rs->getFilePath();
    string fakefullname = GetPathFromFullName(oldfullname) + "noExistDir" + SEP + "noOut.tif";
    EXPECT_FALSE(rs->outputToFile(fakefullname));
    string newfullname = GetPathFromFullName(oldfullname) + "result" + SEP +
                         newcorename + "." + GetSuffix(oldfullname);
    EXPECT_TRUE(rs->outputToFile(newfullname));
}
} /* namespace */