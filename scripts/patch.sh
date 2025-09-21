APP_PATH=/oem/YoudaoDictPen/output
MOD_PATH=/userdata/PenMods
cp $APP_PATH/YoudaoDictPen $APP_PATH/YoudaoDictPen.temp
patchelf --add-needed $MOD_PATH/libPenMods.so $APP_PATH/YoudaoDictPen.temp
mv $APP_PATH/YoudaoDictPen $APP_PATH/YoudaoDictPen.original_bak
mv $APP_PATH/YoudaoDictPen.temp $APP_PATH/YoudaoDictPen

