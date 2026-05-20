::Uncomment "docker builder prune -f" below to clear the build cache
::docker builder prune -f
docker build -t rtcm_trx --progress=plain --no-cache-filter deployment^
 --build-arg PASSPHRASE=ENTER_SPARKFUN_PASSPHRASE_HERE^
 .
docker create --name=rtcm_trx_lora rtcm_trx:latest
docker cp rtcm_trx_lora:/RTCM_TRX.zip .
docker container rm rtcm_trx_lora
