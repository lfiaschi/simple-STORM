/************************************************************************/
/*                                                                      */
/*                  ANALYSIS OF STORM DATA                              */
/*                                                                      */
/*      Copyright 2010-2011 by Joachim Schleicher                       */
/*                                                                      */
/*    Please direct questions, bug reports, and contributions to        */
/*    joachim.schleicher@iwr.uni-heidelberg.de                          */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/

#ifndef STORMPARAMS_H
#define STORMPARAMS_H

#include <string>
#include <vigra/impex.hxx>
#include <vigra/sifImport.hxx>
#ifdef HDF5_FOUND
    #include <vigra/hdf5impex.hxx>
#endif

#define STORMPARAMS_N 3 // could eventually be a template parameter later on

#include "configVersion.hxx"

namespace rude {
class Config;
}

enum FileType { UNDEFINED, TIFF, HDF5, SIF };

class BadConversion : public std::runtime_error {
public:
    BadConversion(std::string const& s)
    : std::runtime_error(s)
    { }
};

class StormParams {
public:
    typedef vigra::MultiArrayShape<STORMPARAMS_N>::type Shape;

    StormParams(int argc, char **argv);
    virtual ~StormParams();

    void printUsage() const;
    void printVersion() const;
    int getFactor() const;
    bool getFactorSaved() const;
    int getRoilen() const;
    bool getRoilenSaved() const;
    float getThreshold() const;
    bool getThresholdSaved() const;
    float getPixelSize() const;
    bool getPixelSizeSaved() const;
    unsigned int getSkellamFrames() const;
    bool getSkellamFramesSaved() const;
    unsigned int getXYChunkSize() const;
    bool getXYChunkSizeSaved() const;
    unsigned int getTChunkSize() const;
    bool getTChunkSizeSaved() const;
    unsigned int getChunksInMemory() const;
    bool getChunksInMemorySaved() const;
    const std::string& getInFile() const;
    const std::string& getOutFile() const;
    const std::string& getCoordsFile() const;
    const std::string& getSettingsFile() const;
    const std::string& getFrameRange() const;
    bool getFrameRangeSaved() const;
    bool getVerbose() const;

    const Shape & shape() const;
    vigra::MultiArrayIndex shape(const int dim) const;
    FileType type() const;
    const std::string& executableDir() const;

    template <typename  T>
    void readVolume(vigra::MultiArrayView<STORMPARAMS_N, T> &) const;
    template <typename  T>
    void readBlock(const Shape&, const Shape&, vigra::MultiArrayView<STORMPARAMS_N, T>&) const;

    virtual void save() const;
    void load();

    mutable void * ptr; // hack

protected:
    mutable rude::Config *m_config;

private:
    int parseProgramOptions(int argc, char **argv);
    void setDefaults();

    Shape m_shape;
    FileType m_type;
    std::string m_executableDir;
    std::string m_executableName;
    int m_factor;
    bool m_factorSaved;
    int m_roilen;
    bool m_roilenSaved;
    float m_threshold;
    bool m_thresholdSaved;
    float m_pixelsize;
    bool m_pixelsizeSaved;
    unsigned int m_skellamFrames;
    bool m_skellamFramesSaved;
    unsigned int m_xyChunkSize;
    bool m_xyChunkSizeSaved;
    unsigned int m_tChunkSize;
    bool m_tChunkSizeSaved;
    unsigned int m_chunksInMemory;
    bool m_chunksInMemorySaved;
    bool m_framesSaved;
    bool m_verbose;
    std::string m_infile;
    std::string m_outfile;
    std::string m_coordsfile;
    std::string m_settingsfile;
    std::string m_frames;

    static const std::string s_section;
};

extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, int8_t>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, int16_t>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, int32_t>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, unsigned int8_t>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, unsigned int16_t>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, unsigned int32_t>&) const;
template<>
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, float>&) const;
extern template
void StormParams::readVolume(vigra::MultiArrayView<STORMPARAMS_N, double>&) const;

extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, int8_t>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, int16_t>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, int32_t>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, unsigned int8_t>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, unsigned int16_t>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, unsigned int32_t>&) const;
template<>
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, float>&) const;
extern template
void StormParams::readBlock(const StormParams::Shape&,
                const StormParams::Shape&,
                vigra::MultiArrayView<STORMPARAMS_N, double>&) const;

#endif // STORMPARAMS_H