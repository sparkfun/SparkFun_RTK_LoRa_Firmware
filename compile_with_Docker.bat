::Uncomment "docker builder prune -f" below to clear the build cache
::docker builder prune -f
docker build -t rtcm_trx --progress=plain --no-cache-filter deployment^
 --build-arg STM32CUBECLT_DIR=stm32cubeclt^
 --build-arg STM32CUBECLT_ZIP=st-stm32cubeclt_1.21.0_27995_20260219_1804_amd64.deb_bundle.sh.zip.gpg^
 --build-arg STM32CUBECLT_VERSION=1.21.0^
 --build-arg PASSPHRASE=ENTER_SPARKFUN_PASSPHRASE_HERE^
 .
docker create --name=rtcm_trx_lora rtcm_trx:latest
docker cp rtcm_trx_lora:/RTCM_TRX.zip .
docker container rm rtcm_trx_lora
