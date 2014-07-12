#####################################
######X-Control Enhanced Plugin######
#####################################
This multi-platform X-Plane plugin provides enhanced controls for the flight simulator. It supports Saitek x52 and x52 Pro joysticks allowing to customize additional data pages on the Multi-Functional Display (MFD) with up to four configurable buttons. 
Additionally, it comes with an embedded realistic FMS capable of calculating expected enroute time (considering realistic climb/descend), required fuel to load, top of climb/ descend distance, time and waypoint. 
The FMS continuously monitors your position and leverages X-Plane voice to brief you regarding the flight plan status, position report and delays from scheduled and expected time.
Even without a supported joystick, both the FMS and a virtual MFD are made available.

###### Index ######
1. Supported Platforms
2. Features
3. Installation
	3.1 Installing the plugin
		3.1.1 Mac OSX
		3.1.2 Linux
		3.1.3 Windows
	3.2 Upgrading the plugin
	3.3 Migrating from x52control
4. Configuration
	4.1 Quick-start
	4.2 Customizing the MFD
	4.3 Customizing the FMS
	4.4 Available MFD pages
5. Troubleshooting
	5.1 The plugin does not load
	5.2 The plugin does not work
6. Contributing
	6.1 Source code
	6.2 Bugs and enhancements
	6.3 Where is x52control?
	6.4 Credits

###### 1. Supported Platforms ######
X-Plane 9.x and 10.x:
- Mac OSX (32 bit and 64 bit)
- Linux (32 bit and 64 bit)
- Windows (32 bit and 64 bit)

###### 2. Features ######
The plugin supports Saitek x52 and x52 Pro joysticks allowing to customize additional data pages on the Multi-Functional Display (MFD) with up to four configurable buttons. 
Additionally, it comes with an embedded realistic FMS. Once a valid flight plan is loaded in the FMS and the plugin is configured with scheduled departure time, cruise ground speed and altitude, it will provide the following information:
	- Expected Enroute Time (considering realistic climb/descend time)
	- Total Distance
	- Required fuel to load
	- Top of Climb/ Top of Descend distance and time
	- For each entry: leg time, leg duration, distance from departure, distance from arrival, scheduled time, expected altitude
Additionally, the FMS continuously monitors your position and keeps track of the following information:
	- Time at departure/arrival gate
	- For each entry actual time/altitude and delay from scheduled and expected time
The plugin also leverages X-Plane voice to brief you regarding the flight plan status and position report.

###### 3. Installation ######
###### 3.1 Installing the plugin ######
###### 3.1.1 Mac OSX ######
- Unzip the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
###### 3.1.2 Linux ######
- Unzip the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- As root, copy the "90-saitekX52.rules" file from the "libs" directory from the plugin folder into "/etc/udev/rules.d" 
- As root, run "/etc/init.d/udev reload"
###### 3.1.3 Windows ######
- Unzip the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- Download and install Visual C++ Redistributable Packages for Visual Studio 2013 from http://www.microsoft.com/en-us/download/details.aspx?id=40784. If running X-Plane 64 Bit, download and install the "vcredist_x64.exe" file, if running X-Plane 32 Bit, download and install the "vcredist_x32.exe" file
- Connect your joystick
- Install Libusb-win32 by running the "libusb-win32-devel-filter-1.2.6.0.exe" file from the "libs" directory from the plugin folder. 
- Upon completion, proceed with running the wizard
- Select your joystick (for Saitek, it is the one with vid:06a3) and click on "Install". This will install the filter driver
- Open the start menu run "Test (Win) program" under "Programs\LibUSB-Win32" or locate the directory "C:\Program Files\LibUSB-Win32\bin" and run "testlibusb-win.exe" to ensure you can see the model of your joystick
	
###### 3.2 Upgrading the plugin ######
- uncompress the package
- Copy and overwrite the directories "32" and "64" from the upgrade package into "x-plane_directory/Resources/plugins/xcontrol" to maintain the existing configuration settings

###### 3.3 Migrating from x52control ######
- uncompress the package
- Copy the entire "xcontrol" directory into "x-plane_directory/Resources/plugins"
- Copy the "x52control.ini" and "x52fms.ini" files from "x-plane_directory/Resources/plugins/x52control" into "x-plane_directory/Resources/plugins/xcontrol" and rename them into "xcontrol.ini" and "xfms.ini"
- Delete the existing x52control installation by moving the directory "x-plane_directory/Resources/plugins/x52control" away from the plugin directory

###### 4. Configuration ######
###### 4.1 Quick-start ######
- Connect the Joystick and run X-Plane
- Configure the buttons (on the joystick or not) to make the MFD cycling between different pages. Go to "Joystick & Equipment", choose the "Buttons: Adv" tab, press the  button you want to use for switching between different MFD pages, select "instruments" from the box in the middle and "xcontrol_cycle_button_1" from the box on the right
- Do the same for "xcontrol_cycle_button_2", "xcontrol_cycle_button_3" , "xcontrol_cycle_button_4" 
- Click the configured buttons to see the MFD changing. Click the same button more then once to cycle among the pages
- Load or configure a valid flight plan
- Set scheduled departure time, cruise ground speed and altitude in the menu "Plugin", "XControl", "FMS Settings"
- While flying, track your progress along the plan by leveraging the MFD or with the summary available in the menu "Plugin", "XControl", "Show FMS Status" (accessible also with the hotkey CTRL+Shift+B)
- The same information printed on the MFD is also available on the virtual MFD available in the menu "Plugin", "XControl", "Show virtual MFD" (accessible also with the hotkey CTRL+Shift+V)
- Optionally edit the xcontrol.ini file in the plugin directory if you want to customize the MFD pages to display (see below "xcontrol.ini configuration")
- Optionally edit the xfms.ini file in the plugin directory if you want to customize the FMS settings (see below "xfms.ini configuration")

###### 4.2 Default MFD configuration ######
By default, the plugin maps the following templates to the configured buttons:
- Button 1 - Navigation information: flight.high,flight.low,fms.status
- Button 2 - Weather information: weather.wind,weather.cloud,weather.condition
- Button 3 - FMS planning information: fms.planner,fms.scheduler
- Button 3 - Radio information: std.radio.com,std.radio.nav,std.radio.adf
For details regarding the templates and the other available pages, see "Available MFD pages".
For changing the pages you want to display, see "Customizing the plugin".

###### 4.3 Customizing the MFD  ######
The xcontrol.ini file can be used to customize buttons, leds and MFD pages. Open the file with a text editor; instructions for each directive are provided within the file. Default values should be fine. 
If you wish to customize for each button the MFD pages to display and cycle within, add the name of the MFD template to the directives mfdpages_button1, mfdpages_button2, mfdpages_button3, mfdpages_button4 (e.g. mfdpages_button1#flight.fms,flight.combined).
For a complete list of the available MFD data pages, see below ("Available Data Pages").

###### 4.4 Customizing the FMS ######
The xms.ini file can be used to customize FMS settings used by the plugin to calculate times and distances. Open the file with a text editor; instructions for each directive are provided within the file.
Planned cruise Flight Level, Ground Speed and expected takeoff time can be configured from the plugin menu and there is no need to change the file for those. Climbing/descending profiles are used to calculate top of climb/descend. Each profile contains the time and distance for different Flight Levels (e.g. how many minutes/miles to climb to the specific altitude/descending from the specific altitude). 
The reason why the plugin uses climb/descend profiles is because they are easy to calculate: start a test flight and calculate the time required to reach FL100,FL200,FL300,FL400,Fl500,Fl600 and the distance travelled as well. Do the same while descending. You now have all the information required to create the profile for your aircraft.
Customizing the fuel profile is easy as well. Track the consumption on ground (taxi,takeoff,landing), for climbing/descending, cruise consumption for each 100nm, IFR/VFR contingency and configure them within this file.
Since some of those values can be aircraft specific, the plugin looks also for a xfms.ini file into the loaded aircraft folder, if found, it takes precedence over the global file. You may need to reload the plugin to have this to work.
	
##### 4.5 Available MFD pages ######
The plugin can be instructed to print on the MFD relevant information coming from the flight simulator. To facilitate the configuration, a number of ready-to-use templates are provided and can be mapped to one or more buttons.
A list of all the available data pages follows below. For each page the template is also provided to give an idea of what is going to be displayed on the screen when selected.

Flight navigation pages:
- flight.combined:
	S:[IAS] M:[match_speed] T:[trasponder]  
	FL:[FL] V:[vs]00 [gear]
	[nav1_id] [nav1_dme]nm F:[flap]
- flight.low
    [IAS] R:[rain_perc]-[runway_friction] V:[visibility] [trasponder]
	FL:[FL] V:[vs] F:[flap]
	[cur_nav_id] [nav_dme] [wind_dir]/[wind_speed]
- flight.high:
	[IAS] [TAS]/[GS] [mach]
	FL:[FL] T:[temp] W:[wind_speed]
	[cur_nav_id] [nav_dme]nm [hh]:[mm]

FMS pages:
- fms.status (next entries):
	[arrival_apt] [remaining_distance] [fuel_eta]
	[hh_eta]:[mm_eta] [id_name]
	[distance]/[remaining]m [hh_time]:[mm_time]
- fms.status (previous entries):	
	[id_name] [time_actual]
	[time_expected] ([delay_from_scheduled]/[delay_from_expected])
	[altitude_expected] ([altitude_actual])
- fms.planner:
	[departure_apt]->[arrival_apt] [total_distance]
	D:[EET] F:[required_fuel_lb]
	TD:[miles] to [FIX_top_of_descend]
- fms.scheduler:
	T:[expected_takeoff_time] ([actual_takeoff_time])
	C:[expected_top_of_climb] D:[expected_top_of_desc]
	T:[expected_landing_time] ([actual_landing_time])

Weather pages: 
- weather.wind:
    W:F[fl1] [wind_dir]/[wind_speed]:[turbulence]T
	W:F[fl2] [wind_dir]/[wind_speed]:[turbulence]T
	W:F[fl3] [wind_dir]/[wind_speed]:[turbulence]T
- weather.cloud:
	C:F[fl1_base]-[fl1_top] [clouds]
	C:F[fl2_base]-[fl2_top] [clouds]
	C:F[fl3_base]-[fl3_top] [clouds]
- weather.condition:
	T:[ground_temp]C [wing_temp]C [outside_temp]C
	W:[ground_wind_dir]/[ground_wind_speed] [outside_wind_dir]/[outside_wind_speed]
	V:[visibility]k R:[rain_level] RWY:[rwy_friction] 

Radio pages:
- std.radio.nav
	Nav1      Nav2
	[radio_active_nav1] Ac [radio_active_nav2]
	[radio_standbay_nav1] Sb [radio_standbay_nav2]
- std.radio.com
	Com1      Com2
	[radio_active_com1] Ac [radio_active_com2]
	[radio_standbay_com1] Sb [radio_standbay_com2]
- std.radio.adf
	Adf1      Adf2
	[radio_active_adf1] Hz Ac [radio_active_adf2] Hz
	[radio_standbay_adf1] Hz Sb [radio_standbay_adf2] Hz
- std.navigation
	hdg:   [hdg]
	spd:  [speed]
	fl :   [flight_level]

###### 5. Troubleshooting ######
###### 5.1 The plugin does not load ######
If the plugin does not load correctly (there is no entry in the Plugins menu and the Joystick's leds do not light up):
- Review the Log.txt file in the X-Plane root directory, searching fro "xcontrol"
- The error message "Error Code = 126 : The specified module could not be found." implies a required library is missing
- If running on Windows, run the "Test (Win) program" from libusb-win32 to ensure the filter driver has been installed correctly
- The plugin is delivered as a "fat" plugin; this means that the correct plugin for your platform/architecture is automatically loaded by X-Plane. However, old X-Plane 9.x versions do not support this feature; so if the plugin is not loading, manually copy the correct xpl file out from the "32" or "64" directory into the plugin root directory.

###### 5.2 The plugin does not work ######
If the plugin loads correctly but the MFD on the joystick is not responding as expected:
- Set "debug" to 1 in the "xcontrol.ini" file in the plugin directory and review the "xlog.txt" file in the plugin directory
- If the joystick is plugged on a different port, you need to run the Libusb-win32 "Install Wizard" from the "Programs" start menu again
	
###### 6. Contributing ######
###### 6.1 Source code ######
The source code is available on GitHub at the following location: https://github.com/user2684/xcontrol
Feel free to contribute.

###### 6.2 Bugs and enhancements ######
Bugs and enhancement requests can be both submitted at the following location: https://github.com/user2684/xcontrol/issues

###### 6.3 Where is x52control? ######
Xcontrol is an evolution of the legacy x52control plugin. Since it includes a few new features which does not require the joystick, the project has been renamed into xcontrol but still providing the same features of the previous version.
The xcontrol plugin is not compatible with x52control; so before installing, the latter has to be disabled (please read "Migrating from x52control").

###### 6.4 Credits ######
This plugin has been derived from x52control 0.4.0-dt whose source code is available at https://github.com/sothis/x52control/tree/v0.4.0-dt


