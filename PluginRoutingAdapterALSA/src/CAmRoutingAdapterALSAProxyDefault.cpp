/*******************************************************************************
 *  \copyright (c) 2016 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Jens Lorenz, jlorenz@de.adit-jv.com 2016
 *           Mattia Guerra, mguerra@de.adit-jv.com 2016
 *
 *
 *  \copyright The MIT License (MIT)
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 ******************************************************************************/


#include "CAmRoutingAdapterALSAProxyDefault.h"
#include "CAmDLTLogging.h"
#include <cerrno>

using namespace am;

CAmRoutingAdapterALSAProxyDefault::CAmRoutingAdapterALSAProxyDefault(const ra_Proxy_s & proxy)
    :mProxy(proxy), mCnt(0), mpCopyBuffer(NULL), mPb(), mCap()
{
    logAmRaInfo("CAmRoutingAdapterALSAProxyDefault::CAmRoutingAdapterALSAProxyDefault - Asynchronous implementation");
    ra_Prefill_s.mPreFill = NULL;
    CAmRoutingAdapterThread::setThreadName("raa_" + mProxy.pcmSrc);
    CAmRoutingAdapterThread::setThreadSched(mProxy.cpuScheduler.policy, mProxy.cpuScheduler.priority);
}

CAmRoutingAdapterALSAProxyDefault::~CAmRoutingAdapterALSAProxyDefault()
{
    stopStreaming();
    closeStreaming();
}

am_Error_e CAmRoutingAdapterALSAProxyDefault::openStreaming()
{
    logAmRaInfo("CRaALSAProxyDefault::openStreaming from", mProxy.pcmSrc, "to", mProxy.pcmSink, this);
    mPb.name = mProxy.pcmSink.c_str();
    mCap.name = mProxy.pcmSrc.c_str();

    return am_Error_e::E_OK;
}

am_Error_e CAmRoutingAdapterALSAProxyDefault::startStreaming()
{
    logAmRaInfo("CRaALSAProxyDefault::startStreaming", this);
    startThread();
    return am_Error_e::E_OK;
}

am_Error_e CAmRoutingAdapterALSAProxyDefault::stopStreaming()
{
    logAmRaInfo("CRaALSAProxyDefault::stopStreaming", this);
    CAmRoutingAdapterThread::stopThread();
    return am_Error_e::E_OK;
}

am_Error_e CAmRoutingAdapterALSAProxyDefault::closeStreaming()
{
    logAmRaInfo("CRaALSAProxyDefault::closeStreaming", this);
    CAmRoutingAdapterThread::joinThread();
    return am_Error_e::E_OK;
}

int CAmRoutingAdapterALSAProxyDefault::initThread()
{
    logAmRaInfo("CRaALSAProxyDefault::initThread", this);
    int err = 0;
    if ((err = snd_pcm_open(&mPb.hndl, mPb.name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0)
    {
        logAmRaError("Can't open", mPb.name, "playback device.", snd_strerror(err));
        return -EFAULT;
    }
    /* Open the PCM device in capture mode */
    if ((err = snd_pcm_open(&mCap.hndl, mCap.name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0)
    {
        logAmRaError("Can't open", mCap.name, "capture device.", snd_strerror(err));
        return -EFAULT;
    }

    /* Configure hardware */
    if ((err = setHwParams(mPb)) < 0)
    {
        return -EFAULT;
    }
    if ((err = setHwParams(mCap)) < 0)
    {
        return -EFAULT;
    }
    if ((err = setHwBuffSize(mPb)) < 0)
    {
        return -EFAULT;
    }
    if ((err = setHwBuffSize(mCap, false)) < 0)
    {
        return -EFAULT;
    }
    if ((err = snd_pcm_hw_params(mCap.hndl, mCap.hwPar)) < 0)
    {
        logAmRaError("CRaALSAProxyDefault::WorkerThread Unable to set hw params for", mCap.name,
                            ":", snd_strerror(err));
        return -EFAULT;
    }
    if ((err = snd_pcm_hw_params(mPb.hndl, mPb.hwPar)) < 0)
    {
        logAmRaError("CRaALSAProxyDefault::WorkerThread Unable to set hw params for", mPb.name,
                            ":", snd_strerror(err));
        return -EFAULT;
    }
    if ((err = setSwParams(mPb)) < 0)
    {
        return -EFAULT;
    }

    /* Allocate buffer to hold single period */
    err = snd_pcm_format_size((snd_pcm_format_t)mProxy.format, 1);
    size_t copyBufSize = mPerSize * mProxy.channels * err;
    mpCopyBuffer = (char*)new char[copyBufSize];

    /* Allocate buffer to hold prefill feature */
    ra_Prefill_s.mPerSize = (mProxy.rate * mProxy.msPrefill) / 1000;
    ra_Prefill_s.prefillByteSize = ra_Prefill_s.mPerSize * mProxy.channels * err;
    if (0 != createPrefill())
    {
        return -EFAULT;
    }

    if (mpCopyBuffer == NULL)
    {
        logAmRaError("CRaALSAProxyDefault::WorkerThread Allocation error!");
        return -EFAULT;
    }

    /* start streaming */
    if ((err = snd_pcm_prepare(mPb.hndl)) < 0)
    {
        logAmRaError("CRaALSAProxyDefault::WorkerThread Unable to prepare", mPb.name,
                            ":", snd_strerror(err));
        return -EFAULT;
    }



    if ((err = snd_pcm_start(mCap.hndl)) < 0)
    {
        logAmRaError("CRaALSAProxyDefault::WorkerThread Unable to start", mCap.name,
                            ":", snd_strerror(err));
        return -EFAULT;
    }
    return 0;
}

int CAmRoutingAdapterALSAProxyDefault::createPrefill()
{
    if (NULL == ra_Prefill_s.mPreFill)
    {
        ra_Prefill_s.mPreFill = (char *)new char[ra_Prefill_s.prefillByteSize];
        if (NULL == ra_Prefill_s.mPreFill)
        {
            logAmRaError("CRaALSAProxyDefault::createPrefill Allocation error!");
            return -ENOMEM;
        }
        else
        {
            memset(ra_Prefill_s.mPreFill, 0, ra_Prefill_s.prefillByteSize);
        }
    }
    return 0;
}

void CAmRoutingAdapterALSAProxyDefault::destroyPrefill()
{
    if (NULL != ra_Prefill_s.mPreFill)
    {
        delete ra_Prefill_s.mPreFill;
    }
    ra_Prefill_s.mPreFill = NULL;
}

int CAmRoutingAdapterALSAProxyDefault::readFromDevice(ap_data_t &device, void *buffer, int sizeOfBuffer)
{
    int err = snd_pcm_wait(device.hndl, 2 * mPerSize * 1000 / mProxy.rate); // timeout should be evaluated in respect of period size, let's say twice for certainty
    if (err == 1)
    {
        err = snd_pcm_readi(device.hndl, buffer, sizeOfBuffer);
        /*
         * If an error occurs, try and recover the device
         */
        if (err < 0)
        {
            err = prepareWithPrefill(device);
        }
    }
    else if (err <= 0)
    {
        /* Restart the PCM to prevent LR channel shift */
        err = prepareWithPrefill(device);
    }
    return err;
}

int CAmRoutingAdapterALSAProxyDefault::writeToDevice(ap_data_t &device, void *buffer, int sizeOfBuffer)
{
    int err = snd_pcm_wait(device.hndl, 2 * mPerSize * 1000 / mProxy.rate); // timeout should be evaluated in respect of period size, let's say twice for certainty
    if (err == 1)
    {
        err = snd_pcm_writei(device.hndl, buffer, sizeOfBuffer);
        /*
         * If an error occurs, try and recover the device
         */
        if (err < 0)
        {
            err = prepareWithPrefill(device);
        }
    }
    else if (err <= 0)
    {
        /* Restart the PCM to prevent LR channel shift */
        err = prepareWithPrefill(device);
    }
    return err;
}

int CAmRoutingAdapterALSAProxyDefault::prepareWithPrefill(ap_data_t &device)
{
    int err = 0;

    /*
     * Sequence for recovering:
     * 1) Drop/Stop the device
     * 2) Create Prefill injecting silence for syncing aim on next writing operation
     * 3) Prepare the device
     * 4) Start the device manually if it's Capture Device
     */

    snd_pcm_drop(device.hndl);
    err = createPrefill();
    if (err < 0)
    {
        return err;
    }
    err = snd_pcm_prepare(device.hndl);
    /* Start device manually only when is Capture device.
     * Playback device will be started automatically when the start_threshold
     * is reached.
     */
    if (err == 0 && device.hndl == mCap.hndl)
    {
        err = snd_pcm_start(device.hndl);
    }
    return err;
}

int CAmRoutingAdapterALSAProxyDefault::workerThread()
{
    int err;

    err = readFromDevice(mCap, mpCopyBuffer, mPerSize);
    if (NULL != ra_Prefill_s.mPreFill)
    {
        writeToDevice(mPb, ra_Prefill_s.mPreFill, (mProxy.rate * mProxy.msPrefill) / 1000);
        destroyPrefill();
    }
    if (err > 0)
    {
        err = writeToDevice(mPb, mpCopyBuffer, err);
    }
    mCnt++;

    return 0;
}

void CAmRoutingAdapterALSAProxyDefault::deinitThread(int errInit)
{
    logAmRaInfo("CRaALSAProxyDefault::deinitThread", this);
    (void)errInit;
    if (mPb.hndl)
    {
        snd_pcm_drop(mPb.hndl);
        snd_pcm_close(mPb.hndl);
    }
    if (mCap.hndl)
    {
        snd_pcm_drop(mCap.hndl);
        snd_pcm_close(mCap.hndl);
    }
    if (mPb.hwPar)
    {
        snd_pcm_hw_params_free(mPb.hwPar);
    }
    if (mCap.hwPar)
    {
        snd_pcm_hw_params_free(mCap.hwPar);
    }
    if (mpCopyBuffer)
    {
        delete(mpCopyBuffer);
    }
    destroyPrefill();
}

int CAmRoutingAdapterALSAProxyDefault::setHwParams(ap_data_t & data)
{
    int err;
    unsigned int rrate;
    snd_pcm_format_t format = static_cast<snd_pcm_format_t>(mProxy.format);
    snd_pcm_hw_params_malloc(&data.hwPar);
    err = snd_pcm_hw_params_any(data.hndl, data.hwPar);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Broken configuration for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_set_access(data.hndl, data.hwPar, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Access type not available for",
                            data.name,":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_set_format(data.hndl, data.hwPar, format);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Sample format not available for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    int channels = mProxy.channels;
    err = snd_pcm_hw_params_set_channels(data.hndl, data.hwPar, channels);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Channels count", channels, "not available for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    rrate = mProxy.rate;
    err = snd_pcm_hw_params_set_rate_near(data.hndl, data.hwPar, &rrate, 0);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Rate", rrate, "Hz not available for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    if (rrate != mProxy.rate)
    {
        logAmRaError("CRaALSAProxyDefault::setHwParams Rate doesn't match (requested",
                            mProxy.rate, "Hz, got", rrate, "Hz)");
        return -EINVAL;
    }
    return 0;
}

bool getNearest(snd_pcm_uframes_t & val, snd_pcm_uframes_t min, snd_pcm_uframes_t max)
{
    if ((min == max) && (min != val))
    {
        val = min;
        return true;
    }
    else if (min > val)
    {
        val = min;
        return true;
    }
    else if (max < val)
    {
        val = max;
        return true;
    }
    else if ((min != 0) && (max == 0))
    {
        val = min;
        return true;
    }
    else if ((min == 0) && (max != 0))
    {
        val = min;
        return true;
    }
    return false;
}

int CAmRoutingAdapterALSAProxyDefault::setHwBuffSize(ap_data_t & data, bool firstTry)
{
    int err;
    snd_pcm_uframes_t perSizeMin;
    snd_pcm_uframes_t perSizeMax;
    snd_pcm_uframes_t bufSizeMin;
    snd_pcm_uframes_t bufSizeMax;

    err = snd_pcm_hw_params_get_period_size_min(data.hwPar, &perSizeMin, NULL);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to get min period size for", data.name,
                        ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_period_size_max(data.hwPar, &perSizeMax, NULL);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to get max period size for", data.name,
                            ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_buffer_size_min(data.hwPar, &bufSizeMin);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to get min buffer size for", data.name,
                            ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_buffer_size_max(data.hwPar, &bufSizeMax);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to get max buffer size for", data.name,
                            ":", snd_strerror(err));
        return err;
    }

    if (firstTry)
    {
        err = snd_pcm_format_size((snd_pcm_format_t)mProxy.format, 1);
        if (err < 0)
        {
            logAmRaError("CRaALSAProxyDefault::setHwBuffSize Format", mProxy.format, "for", data.name,
                                "is unsupported");
            return err;
        }

        snd_pcm_uframes_t perSize = (mProxy.rate * mProxy.msBuffersize) / 1000;
        mPerSize = perSize;
        if (getNearest(mPerSize, perSizeMin, perSizeMax))
        {
            logAmRaInfo("CRaALSAProxyDefault::setHwBuffSize Period size for", data.name,
                                "changed from", (uint32_t)perSize, "to", (uint32_t)mPerSize);
        }

        mBufSize = mPerSize * 3;
        if (getNearest(mBufSize, bufSizeMin, bufSizeMax))
        {
            logAmRaInfo("CRaALSAProxyDefault::setHwBuffSize Buffer size for", data.name,
                                "changed to", (uint32_t)mBufSize);
        }
        if (mBufSize < (mPerSize * 2))
        {
            logAmRaError("CRaALSAProxyDefault::setHwBuffSize Buffer of device", data.name,
                                "is too small");
            return -EINVAL;
        }
    }
    else
    {
        if (getNearest(mPerSize, perSizeMin, perSizeMax) ||
            getNearest(mBufSize, bufSizeMin, bufSizeMax))
        {
            logAmRaError("CRaALSAProxyDefault::setHwBuffSize Hardware configurations incompatible");
            return -EINVAL;
        }
    }
    err = snd_pcm_hw_params_set_buffer_size_near(data.hndl, data.hwPar, &mBufSize);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to set buffer size", (uint32_t)mBufSize,
                            "for", data.name, ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_set_period_size_near(data.hndl, data.hwPar, &mPerSize, NULL);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setHwBuffSize Unable to set period size", (uint32_t)mPerSize,
                            "for", data.name, ":", snd_strerror(err));
        return err;
    }

    return 0;
}

int CAmRoutingAdapterALSAProxyDefault::setSwParams(ap_data_t & data)
{
    int err;

    snd_pcm_sw_params_alloca(&data.swPar);
    err = snd_pcm_sw_params_current(data.hndl, data.swPar);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setSwParams Unable to determine current swparams for",
                            data.name, ":", snd_strerror(err));
        return err;
    }

    err = snd_pcm_sw_params_set_start_threshold(data.hndl, data.swPar, mBufSize - mPerSize);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setSwParams Unable to set start threshold mode for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_sw_params_set_avail_min(data.hndl, data.swPar, mPerSize);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setSwParams Unable to set avail min for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    err = snd_pcm_sw_params(data.hndl, data.swPar);
    if (err < 0)
    {
        logAmRaError("CRaALSAProxyDefault::setSwParams Unable to set for",
                            data.name, ":", snd_strerror(err));
        return err;
    }
    return 0;
}

am_timeSync_t CAmRoutingAdapterALSAProxyDefault::getDelay() const
{
    return static_cast<am_timeSync_t>(mProxy.msPrefill + mProxy.msBuffersize);
}
