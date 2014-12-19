Camera and LED calibration

There are several applications that must be used in the process of calibrating both the cameras and the LED positions. The process of calibration is described here.


LedCalibration

The LedCalibration application calibrates cameras and LEDs. It creates a directory hierarchy as follows:

- project_folder
  - calibration.calib
  - images
    - camcalib
       - 0.jpg
       - 1.jpg
       - 2.jpg
       ...
    - LEDcalib
       - 0.jpg
       - 1.jpg
       - 2.jpg
       ...

The project folder must be empty when a new project is created. If an existing project is loaded at least the calibration.calib file must exist. The image sub folders are optional. Those subfolders contain jpg images of the collected samples.

The contents of the calibration.calib project file are:

    <camera>
        <intr>                 intrinsic matrix, column-major
        <dist>                 distortion coefficients
        <reprojection_error>   reprojection error
        <image_size>           image size
        <object_points>        object points in the objects coordinate system
        <sample id="0">        sample
            <image_points>     extracted 2D coordinates of the markers
            <image_path>       path to the saved JPG image

    <LEDs>
        <LED id="0">
            <led_pos>          3D estimated LED position
            <circle_spacing>   circle spacing in meters
            <object_points>    object points in the objects coordinate system
            <sample id="0">    sample
               <image_points>  extracted 2D coordinates of the markers
               <image_path>    path to the saved JPG image


An example of such file can be found at the end of this documentation.


NOTE: When calibrating the cameras and LEDs, make sure to adjust the focal length, i.e. the auto focus to a constant value and remember it.


Calibrating the cameras


The cameras can be calibrated by "LedCalibration". In order to create a new project, click File->New and select an _empty_ folder. You can also create an empty folder in the file selection dialog. Alternatively you can open an existing project through File->Open. Look for calibration.calib.

You will see that there is a new item in the list view, the name is "new item". Expanding that item reveals the following tree structure:

       new item
       |
       |--Camera
       |  |--intr
       |  |--dist
       |  |--err
       |  |--imgSz
       |
       |--LEDs
          |--LED
             |--cs
             |--pos

First the camera must be calibrated using the pattern in "acircles_pattern.png".
1. Select the device from the drop down
2. Check the calibrate box
3. Show the pattern to the camera and click "Add sample" with varying different poses of the circle board.
4. Once at least 20 samples have been collected (defined in CameraCalibrator.cpp) click "Calibrate".

Note: The calibration is a long process, might be several minutes, during which the GUI does not respond.

At this point it is worth saving the project, File->Save. From this point on any time the user wants to save, the application will ask permission to overwrite the existing data. Click "Ok". The application will never auto save, nor does it prompt the user on exit if the project is unsaved. The user must remember to do so manually. It is a good idea to save after collecting data for the cameras or for the LEDs.


Calibrating the LEDs

Once the camera is calibrated, the LEDs can be calibrated. There will always be one LED item visible although the user has not added any. Before calibration, the circle spacing (cs) must be set by double clicking that item. The unit is whatever, but remember that all gaze tracking algorithms assume meters, so the user is best off choosing meter as the unit. It is very important to adjust the light power to the minimum of the LED in question, to ensure a small uniform circle. That the way the estimate of the LED centre is more accurate. The other LEDs may be adjusted to levels that ensure the best background lighting in the sample collection process.

1. Choose the circle spacing by double clicking the item (cs)
2. click the LED item to activate it, it will appear blue.
3. Check the calibrate box
4. If you want, check the "Gray" box as well to see a thresholded image (poor naming)
5. Show the pattern (LEDCalibration/LEDCalibPattern.pdf) and collect samples.
6. Click calibrate to view your results (led pos).
7. Click save

Insert the next LED by right clicking the LEDs item. Repeat steps 1-7 for this and the following LEDs. Remove LEDs by right clicking the LED items and selecting "Delete LED". Note that there will always be that one LED in the list. The order in which the LEDs are calibrated is arbitrary. However, it is convenient to select the order used in GazeTracker.cpp. The order is such that the LEDs appear counter clock wise on the 2D image. Since the wearable goggles make use of a mirror, calibrating in clock wise order is suggested (looking from the "eyes" direction).

For each collected sample a thumbnail representing that sample will be generated at the bottom of the window. Individual samples can be deleted by selecting one or multiple by left-mouse-clicking the sample (Hold Ctrl to select multiple) and then pressing the delete key.



Inspecting the samples

During the process of collecting the LED samples, it is likely that some samples are bad. Those samples need to be either removed or refined. For this purpose there is a utility application calibrate_LED_images/calibrate. This takes a project file as an input and it writes a refined output project file. For details type "calibrate -h". This program allows for relocating the LED reflection using the mouse or keyboard and deleting the samples.



Flip and calibrate

When tracking the gaze with a system based on the hot mirror approach the samples must be flipped before calibration. This is because the "virtual" camera sees the image fliiped around the y-axis. For this purpose there is a third application calibrate_flipped_samples/calibrator. It reads a project file produced by LedCalibrator, flips the samples and calibrates. It produces a corresponding output project file. See ./calibrator -h for more details.



View the results

There is an application for viewing the results. It builds a 3D scene of the camera and the LEDs and displays them. The user can navigate in this scene. The program is in tests/calib_viewer/samples. See ./samples -help for more details.









Example of calibration.calib


<calibration>
    <camera>
        <intr>753.612175202962,0,0,0,750.998165085514,0,327.198608440361,251.940077742822,1</intr>
        <dist>0.310411540110475,-1.56763345891941,0.00424952629912633,-0.00800702311685595,1.32484818761273</dist>
        <reprojection_error>0.304071</reprojection_error>
        <object_points>(0,0,0),(2,0,0),(4,0,0),(6,0,0),(1,1,0),(3,1,0),(5,1,0),(7,1,0),(0,2,0),(2,2,0),(4,2,0),(6,2,0),(1,3,0),(3,3,0),(5,3,0),(7,3,0),(0,4,0),(2,4,0),(4,4,0),(6,4,0),(1,5,0),(3,5,0),(5,5,0),(7,5,0),(0,6,0),(2,6,0),(4,6,0),(6,6,0),(1,7,0),(3,7,0),(5,7,0),(7,7,0),(0,8,0),(2,8,0),(4,8,0),(6,8,0),(1,9,0),(3,9,0),(5,9,0),(7,9,0),(0,10,0),(2,10,0),(4,10,0),(6,10,0)</object_points>
        <image_size>640,480</image_size>
        <sample id="0">
            <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/camcalib/0.jpg</image_path>
            <image_points>(305.268341064453,163.623764038086),(337.491760253906,147.323196411133),(370.597503662109,130.123352050781),(404.147338867188,112.63224029541),(329.108306884766,172.187026977539),(362.045593261719,155.395385742188),(395.258270263672,138.367385864258),(429.595794677734,120.798797607422),(320.568054199219,196.853561401367),(352.991241455078,180.652099609375),(386.192657470703,163.801849365234),(420.364654541016,146.672988891602),(344.280883789062,205.43717956543),(377.097290039062,189.291473388672),(411.069519042969,172.341094970703),(445.748596191406,155.033615112305),(335.684661865234,230.155410766602),(368.28759765625,214.104949951172),(402.017425537109,197.504699707031),(436.481231689453,180.783584594727),(359.784423828125,238.817611694336),(393.136322021484,222.830108642578),(427.350921630859,206.392959594727),(462.501281738281,189.547882080078),(351.042358398438,263.447479248047),(384.078643798828,247.797225952148),(418.010650634766,231.823181152344),(453.108459472656,215.233947753906),(375.472930908203,272.631896972656),(409.258483886719,256.828552246094),(443.792144775391,240.85807800293),(479.156860351562,224.325485229492),(366.461486816406,297.148864746094),(400.223266601562,281.686676025391),(434.421600341797,266.1904296875),(469.869781494141,249.968246459961),(391.152923583984,306.826812744141),(425.526336669922,291.272155761719),(460.289215087891,275.463592529297),(496.361846923828,259.183532714844),(382.289947509766,331.764678955078),(416.198913574219,316.301910400391),(451.444427490234,300.606262207031),(486.659759521484,284.773345947266)</image_points>
        </sample>
        <sample id="1">
            <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/camcalib/1.jpg</image_path>
            <image_points>(314.937927246094,135.798278808594),(349.442932128906,118.270980834961),(385.015533447266,100.327026367188),(421.068237304688,81.4313888549805),(339.435424804688,145.098968505859),(374.967376708984,126.922874450684),(410.981262207031,108.661636352539),(447.915771484375,90.0043029785156),(329.893920898438,170.912887573242),(365.027191162109,153.298889160156),(400.852996826172,135.480194091797),(437.587799072266,117.176200866699),(354.799438476562,179.885772705078),(390.647521972656,162.434768676758),(427.230590820312,144.433013916016),(464.914031982422,125.959716796875),(345.056030273438,206.24528503418),(380.156951904297,189.224517822266),(416.9443359375,171.415176391602),(454.530639648438,153.185180664062),(370.512359619141,215.658477783203),(406.933776855469,198.315551757812),(443.952575683594,180.870071411133),(482.299957275391,162.595764160156),(360.522521972656,241.802322387695),(396.333160400391,225.197052001953),(433.772277832031,207.93798828125),(471.702209472656,190.248947143555),(386.444213867188,251.994720458984),(423.237243652344,235.0947265625),(461.078765869141,217.904449462891),(500.075500488281,200.112319946289),(376.235687255859,278.278503417969),(412.959991455078,261.977966308594),(450.855010986328,245.289978027344),(489.506713867188,227.852600097656),(402.557800292969,288.804962158203),(440.213439941406,272.508514404297),(478.710113525391,255.440689086914),(518.575012207031,237.932647705078),(392.421173095703,315.895538330078),(429.670715332031,299.533142089844),(468.619873046875,282.698120117188),(507.418731689453,265.918914794922)</image_points>
        </sample>

    </camera>


    <LEDs>

        <LED id="0">
            <led_pos>0.012644990952484,-0.000866046127091078,0.0470068443741578</led_pos>
            <circle_spacing>0.005000</circle_spacing>
            <object_points>(-2,-2,0),(-2,-1,0),(-2,0,0),(-2,1,0),(-2,2,0),(-1,2,0),(0,2,0),(1,2,0),(2,2,0),(2,1,0),(2,0,0),(2,-1,0),(2,-2,0),(1,-2,0),(0,-2,0),(-1,-2,0)</object_points>
            <sample id="0">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/205.jpg</image_path>
                <image_points>(186.449142456055,116.050628662109),(187.135269165039,166.155838012695),(187.703140258789,215.332382202148),(187.967559814453,264.05810546875),(187.720275878906,312.229858398438),(236.938354492188,310.944366455078),(284.33203125,309.958343505859),(330.245330810547,308.909301757812),(375.066650390625,308.228454589844),(375.973297119141,262.688842773438),(376.843994140625,216.811950683594),(377.524108886719,170.336013793945),(378.514770507812,123.109664916992),(332.534088134766,121.454208374023),(285.630340576172,119.821083068848),(236.978713989258,117.976951599121)</image_points>
                <glint_pos>319.94087403599,209.426735218509</glint_pos>
            </sample>
            <sample id="1">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/206.jpg</image_path>
                <image_points>(185.604949951172,115.529884338379),(186.404235839844,165.888061523438),(186.957015991211,215.207778930664),(187.030990600586,263.946716308594),(186.809326171875,312.372344970703),(236.229843139648,310.861602783203),(283.809936523438,309.775543212891),(329.804321289062,308.755432128906),(374.709777832031,307.79833984375),(375.574768066406,262.351318359375),(376.358154296875,216.167251586914),(377.209991455078,169.663497924805),(377.898315429688,122.419548034668),(332.058532714844,120.887718200684),(284.845977783203,119.227546691895),(236.148666381836,117.506904602051)</image_points>
                <glint_pos>319.458333333333,210.350694444444</glint_pos>
            </sample>
            <sample id="2">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/207.jpg</image_path>
                <image_points>(200.043075561523,124.218330383301),(200.880935668945,174.685440063477),(201.337387084961,224.269989013672),(201.510803222656,273.299102783203),(201.329467773438,322.027526855469),(250.70344543457,320.169586181641),(298.255920410156,318.704864501953),(344.172943115234,317.43701171875),(389.018798828125,316.492736816406),(389.416442871094,270.450927734375),(390.053741455078,224.43928527832),(390.868438720703,177.862442016602),(391.549102783203,130.690765380859),(345.957824707031,129.200149536133),(298.989990234375,127.640892028809),(250.618957519531,126.119483947754)</image_points>
                <glint_pos>309.652567975831,216.081570996979</glint_pos>
            </sample>
            <sample id="3">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/208.jpg</image_path>
                <image_points>(197.864852905273,124.598274230957),(198.749725341797,175.079040527344),(199.187530517578,224.759826660156),(199.437850952148,273.919067382812),(199.479248046875,322.842193603516),(249.004852294922,320.974060058594),(296.575317382812,319.539825439453),(342.552856445312,318.246307373047),(387.6259765625,317.145782470703),(387.961395263672,271.158813476562),(388.434692382812,224.696487426758),(389.130950927734,178.238647460938),(389.679748535156,130.899490356445),(344.092712402344,129.423645019531),(296.934600830078,127.989532470703),(248.370330810547,126.398445129395)</image_points>
                <glint_pos>309.391184573003,216.184573002755</glint_pos>
            </sample>

        </LED>
        <LED id="1">
            <led_pos>0.0126496037621794,-0.00938757313979872,0.047519805117629</led_pos>
            <circle_spacing>0.005000</circle_spacing>
            <object_points>(-2,-2,0),(-2,-1,0),(-2,0,0),(-2,1,0),(-2,2,0),(-1,2,0),(0,2,0),(1,2,0),(2,2,0),(2,1,0),(2,0,0),(2,-1,0),(2,-2,0),(1,-2,0),(0,-2,0),(-1,-2,0)</object_points>
            <sample id="0">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/164.jpg</image_path>
                <image_points>(181.329956054688,138.804382324219),(181.918853759766,190.541687011719),(182.452453613281,242.403366088867),(182.412750244141,294.933227539062),(182.205291748047,348.341430664062),(235.850662231445,346.382995605469),(286.944000244141,344.631500244141),(335.92919921875,343.137786865234),(383.126983642578,341.945861816406),(381.5654296875,292.539581298828),(380.156005859375,244.003921508789),(378.62841796875,196.429046630859),(377.297119140625,149.21369934082),(331.337371826172,146.864852905273),(283.700531005859,144.417266845703),(233.906723022461,141.751800537109)</image_points>
                <glint_pos>285.728744939271,233.242914979757</glint_pos>
            </sample>
            <sample id="1">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/165.jpg</image_path>
                <image_points>(181.193634033203,136.188354492188),(181.901596069336,188.344680786133),(182.434875488281,240.664810180664),(182.53791809082,293.806671142578),(182.298110961914,347.614929199219),(236.595993041992,345.616516113281),(288.068603515625,343.985382080078),(337.676239013672,342.49169921875),(385.278503417969,341.207611083984),(383.480651855469,291.387145996094),(382.029876708984,242.421905517578),(380.364135742188,194.508758544922),(379.063903808594,146.850570678711),(332.528289794922,144.463760375977),(284.542602539062,141.925048828125),(234.246047973633,139.291702270508)</image_points>
                <glint_pos>287.466666666667,233.552380952381</glint_pos>
            </sample>
            <sample id="2">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/166.jpg</image_path>
                <image_points>(180.506622314453,135.474960327148),(181.405944824219,189.249404907227),(182.346633911133,243.186508178711),(182.817596435547,297.733856201172),(182.926330566406,353.390808105469),(238.727874755859,351.107635498047),(291.511383056641,349.158935546875),(342.290649414062,347.378387451172),(391.059631347656,346.028381347656),(389.017761230469,294.759979248047),(387.090026855469,244.447418212891),(385.441986083984,194.907730102539),(383.622253417969,145.957046508789),(336.113952636719,143.635986328125),(286.861114501953,141.246963500977),(235.116149902344,138.482925415039)</image_points>
                <glint_pos>295.233463035019,228.587548638132</glint_pos>
            </sample>
            <sample id="3">
                <image_path>/home/sharman/code/C++/Ganzheit/LedCalibration/calib/20130406/images/LEDcalib/167.jpg</image_path>
                <image_points>(186.556381225586,135.578750610352),(187.412368774414,189.482406616211),(188.032440185547,243.357437133789),(188.218475341797,298.164886474609),(187.982315063477,353.913909912109),(243.645812988281,351.730346679688),(296.57763671875,349.787567138672),(347.227416992188,348.131439208984),(396.023895263672,346.822052001953),(394.355590820312,295.176849365234),(392.615173339844,244.687622070312),(391.224700927734,195.200393676758),(389.527404785156,146.027603149414),(341.894165039062,143.656356811523),(292.761383056641,141.311401367188),(241.037384033203,138.539154052734)</image_points>
                <glint_pos>296.698564593301,226.483253588517</glint_pos>
            </sample>

        </LED>

    </LEDs>

</calibration>

