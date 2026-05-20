FROM ubuntu:latest AS upstream

ARG DEBIAN_FRONTEND=noninteractive

# STM32CubeCLT is licensed software. SparkFun's copy is passphrase-encrypted
# https://github.com/sparkfun/SparkFun_STM32CubeCLT
ARG PASSPHRASE

# Path to the repo containing the encrypted STM32CubeCLT Debian Linux Installer zip file. E.g.:
# st-stm32cubeclt_1.21.0_27995_20260219_1804_amd64.deb_bundle.sh.zip.gpg
ARG STM32CUBECLT_REPO_URL=https://github.com/sparkfun/SparkFun_STM32CubeCLT
ARG STM32CUBECLT_REPO_NAME=SparkFun_STM32CubeCLT
# TODO: parse these from the zip filename after the git clone
ARG STM32CUBECLT_ZIP=st-stm32cubeclt_1.21.0_27995_20260219_1804_amd64.deb_bundle.sh.zip.gpg
ARG STM32CUBECLT_VERSION=1.21.0

# Establish Development Environment
RUN apt-get update \
    && apt-get install -y \
        gcc \
        g++ \
        x11-apps \
        libswt-gtk-4-java \
        curl \
        python3 \
        python3-pycryptodome \
        python3-ecdsa \
        python3-pyelftools \
        python3-numpy \
        git \
        git-lfs \
        zip \
        unzip \
        libglib2.0-0 \
        libusb-1.0-0 \
        cmake \
        make \
        udev \
        nano \
        ssh \
        ca-certificates \
        gpg \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Clone the STM32CubeCLT repo - then we can pull the stm32cubeclt zip file from GitHub LFS
RUN git clone $STM32CUBECLT_REPO_URL.git

# TODO: parse STM32CUBECLT_ZIP and STM32CUBECLT_VERSION from the zip filename after the git clone

# Pull the encrypted stm32cubeclt zip file from GitHub LFS
RUN cd $STM32CUBECLT_REPO_NAME \
    && git lfs pull -I $STM32CUBECLT_ZIP \
    && mv $STM32CUBECLT_ZIP /tmp/stm32cubeclt-installer.sh.zip.gpg

# Set environment variables
ENV LICENSE_ALREADY_ACCEPTED=1
ENV TZ=Etc/UTC
ENV PATH="${PATH}:/opt/st/stm32cubeclt_${STM32CUBECLT_VERSION}"
ENV DISPLAY=:0

# Decrypt the gpg file and delete it
RUN gpg --batch --yes --decrypt --passphrase $PASSPHRASE -o /tmp/stm32cubeclt-installer.sh.zip /tmp/stm32cubeclt-installer.sh.zip.gpg \
    && rm /tmp/stm32cubeclt-installer.sh.zip.gpg

# Unzip STM32 Cube CLT and delete zip file
RUN unzip -p /tmp/stm32cubeclt-installer.sh.zip > /tmp/stm32cubeclt-installer.sh \
    && rm /tmp/stm32cubeclt-installer.sh.zip

# Install STM32 Cube CLT and delete installer
RUN chmod +x /tmp/stm32cubeclt-installer.sh \
    && /tmp/stm32cubeclt-installer.sh \
    && rm /tmp/stm32cubeclt-installer.sh

# ===========================================================================================

# Copy RTCM_TRX_FSS_RTK and build deployment image
FROM upstream AS deployment

# mkdir to hold the files
RUN cd /home \
    && mkdir paul \
    && cd paul \
    && mkdir Documents

# Add the source files
ADD . /home/paul/Documents

ENV PATH=/opt/st/stm32cubeclt_$STM32CUBECLT_VERSION/STM32CubeProgrammer/bin:${PATH}
ENV PATH=/opt/st/stm32cubeclt_$STM32CUBECLT_VERSION/GNU-tools-for-STM32/bin:${PATH}
ENV PATH=/opt/st/stm32cubeclt_$STM32CUBECLT_VERSION/STLink-gdb-server/bin:${PATH}
ENV LD_LIBRARY_PATH=/opt/st/stm32cubeclt_$STM32CUBECLT_VERSION/STLink-gdb-server/bin/native/linux_x64

# Run CubeCLT
RUN ["dash", "-c", "\
cd /home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Debug \
&& make -j8 all \
"]

# Rename files and zip
RUN cd /home/paul/Documents \
    && VERSION=$(grep "#define VERSION " ./RTCM_TRX_FSS_RTK/SubGHz_Phy/App/include/app_common.h | cut -c 18-22) \
    && cp ./RTCM_TRX_FSS_RTK/STM32CubeIDE/Debug/RTCM_TRX.elf ./RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_$VERSION.elf \
    && cp ./RTCM_TRX_FSS_RTK/STM32CubeIDE/Debug/RTCM_TRX.hex ./RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_$VERSION.hex \
    && cp ./RTCM_TRX_FSS_RTK/STM32CubeIDE/Debug/RTCM_TRX.bin ./RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_$VERSION.bin \
    && rm -f ./RTCM_TRX.zip \
    && zip ./RTCM_TRX.zip ./RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_* \
    && rm ./RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_$VERSION.*

# ===========================================================================================

# Copy the zipped compile output. List the files
FROM deployment AS output

COPY --from=deployment /home/paul/Documents/RTCM_TRX.zip /

CMD echo $(ls /*.*)

