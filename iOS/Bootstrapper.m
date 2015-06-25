/*
 * This file is part of MAME4iOS.
 *
 * Copyright (C) 2013 David Valdeita (Seleuco)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 *
 * Linking MAME4iOS statically or dynamically with other modules is
 * making a combined work based on MAME4iOS. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of MAME4iOS
 * give you permission to combine MAME4iOS with free software programs
 * or libraries that are released under the GNU LGPL and with code included
 * in the standard release of MAME under the MAME License (or modified
 * versions of such code, with unchanged license). You may copy and
 * distribute such a system following the terms of the GNU GPL for MAME4iOS
 * and the licenses of the other code concerned, provided that you include
 * the source code of that other code when and as the GNU GPL requires
 * distribution of source code.
 *
 * Note that people who make modified versions of MAME4iOS are not
 * obligated to grant this special exception for their modified versions; it
 * is their choice whether to do so. The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception.
 *
 * MAME4iOS is dual-licensed: Alternatively, you can license MAME4iOS
 * under a MAME license, as set out in http://mamedev.org/
 */

#import <UIKit/UIKit.h>

#import "Bootstrapper.h"
#import "Globals.h"
#import "BTJoyHelper.h"

#include <sys/stat.h>

#define IS_WIDESCREEN ( fabs( ( double )[ [ UIScreen mainScreen ] bounds ].size.height - ( double )568 ) < DBL_EPSILON )
#define IS_IPAD   ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPad" ] )
#define IS_IPHONE ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPhone" ] )
#define IS_IPOD   ( [ [ [ UIDevice currentDevice ] model ] isEqualToString: @"iPod touch" ] )
#define IS_IPHONE_5 ( IS_IPHONE && IS_WIDESCREEN )

const char* get_resource_path(const char* file)
{
    static char resource_path[1024];
    
#ifdef JAILBREAK
    sprintf(resource_path, "/Applications/MAME4iOS.app/%s", file);
#else
    const char *userPath = [[[NSBundle mainBundle] bundlePath] cStringUsingEncoding:NSASCIIStringEncoding];
    sprintf(resource_path, "%s/%s", userPath, file);
#endif
    return resource_path;
}

const char* get_documents_path(const char* file)
{
    static char documents_path[1024];
    
#ifdef JAILBREAK
    sprintf(documents_path, "/var/mobile/Media/ROMs/MAME4iOS/%s", file);
#else
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	const char *userPath = [[paths objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding];
    sprintf(documents_path, "%s/%s",userPath, file);
#endif
    return documents_path;
}

@implementation Bootstrapper

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification {

	struct CGRect rect = [[UIScreen mainScreen] bounds];
	rect.origin.x = rect.origin.y = 0.0f;
	
	//printf("Machine: '%s'\n",[[Helper machine] UTF8String]) ;
    NSError *error;
    NSString *fromPath,*toPath;
    NSFileManager* manager = nil;

#ifdef JAILBREAK
    manager = [[NSFileManager alloc] init];
    if(![manager fileExistsAtPath:[NSString stringWithUTF8String:get_documents_path("")]] )
    {
        
        mkdir("/var/mobile/Media/ROMs/", 0755);
        if(mkdir(get_documents_path(""), 0755) != 0)
        {
        
           UIAlertView *errAlert = [[UIAlertView alloc] initWithTitle:@"Error"
                                                           message:
                                 @"Directory cannot be created!\nCheck for write permissions.\n'chmod -R 777 /var/mobile/Media/ROMs' if needed.\nLook at the help for details."
                                                          delegate:nil
                                                 cancelButtonTitle:@"Dismiss"
                                                 otherButtonTitles: nil];
          [errAlert show];
          [errAlert release];
        }
    }
    
    [manager release];
#endif

    int r = chdir (get_documents_path(""));
	printf("running... %d\n",r);
    
    mkdir(get_documents_path("iOS"), 0755);
	mkdir(get_documents_path("artwork"), 0755);
	mkdir(get_documents_path("cfg"), 0755);
	mkdir(get_documents_path("nvram"), 0755);
	mkdir(get_documents_path("ini"), 0755);
	mkdir(get_documents_path("snap"), 0755);
	mkdir(get_documents_path("sta"), 0755);
	mkdir(get_documents_path("hi"), 0755);
	mkdir(get_documents_path("inp"), 0755);
	mkdir(get_documents_path("memcard"), 0755);
	mkdir(get_documents_path("samples"), 0755);
	mkdir(get_documents_path("roms"), 0755);
    

#ifndef JAILBREAK
    [[NSURL fileURLWithPath: [NSString stringWithUTF8String:get_documents_path("roms")]]
     setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:nil];
    [[NSURL fileURLWithPath: [NSString stringWithUTF8String:get_documents_path("artwork")]]
     setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:nil];
    [[NSURL fileURLWithPath: [NSString stringWithUTF8String:get_documents_path("samples")]]
     setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:nil];
    [[NSURL fileURLWithPath: [NSString stringWithUTF8String:get_documents_path("nvram")]]
     setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:nil];
    [[NSURL fileURLWithPath: [NSString stringWithUTF8String:get_documents_path("cheat.zip")]]
     setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:nil];
#endif  
    
    manager = [[NSFileManager alloc] init];
    
    fromPath = [NSString stringWithUTF8String:get_resource_path("gridlee.zip")];
    toPath = [NSString stringWithUTF8String:get_documents_path("roms/gridlee.zip")];

    if([manager fileExistsAtPath:fromPath] && ![manager fileExistsAtPath:toPath])
    {
        [manager copyItemAtPath:fromPath toPath:toPath error:nil];
    }
    
    isGridlee = [manager fileExistsAtPath:toPath] && [[manager contentsOfDirectoryAtPath:[NSString stringWithUTF8String:get_documents_path("roms")] error:nil] count] == 1;
	    
    toPath = [NSString stringWithUTF8String:get_documents_path("cheat.zip")];
    if (![manager fileExistsAtPath:toPath] && !isGridlee)
    {
        error = nil;
        fromPath = [NSString stringWithUTF8String:get_resource_path("cheat.zip")];
        [manager copyItemAtPath: fromPath toPath:toPath error:&error];
        NSLog(@"Unable to move file cheat? %@", [error localizedDescription]);
    }
    toPath = [NSString stringWithUTF8String:get_documents_path("Category.ini")];
    if (![manager fileExistsAtPath:toPath] && !isGridlee)
    {
        error = nil;
        fromPath = [NSString stringWithUTF8String:get_resource_path("Category.ini")];
        [manager copyItemAtPath: fromPath toPath:toPath error:&error];
        NSLog(@"Unable to move file category? %@", [error localizedDescription]);
    }
    toPath = [NSString stringWithUTF8String:get_documents_path("hiscore.dat")];
    if (![manager fileExistsAtPath:toPath] && !isGridlee)
    {
        error = nil;
        fromPath = [NSString stringWithUTF8String:get_resource_path("hiscore.dat")];
        [manager copyItemAtPath: fromPath toPath:toPath error:&error];
        NSLog(@"Unable to move file hiscore? %@", [error localizedDescription]);
    }

    
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation : UIStatusBarAnimationNone];
    
    g_isIpad = IS_IPAD;
    g_isIphone5 = IS_WIDESCREEN; //Really want to know if widescreen
    //g_isIphone5 = true; g_isIpad = false;//TEST
    
	hrViewController = [[EmulatorController alloc] init];
	
	deviceWindow = [[UIWindow alloc] initWithFrame:rect];
	deviceWindow.backgroundColor = [UIColor redColor];
    
	//[deviceWindow addSubview: hrViewController.view ];//LO CAMBIO PARA QUE GIRE EN iOS 6.0	
    [deviceWindow setRootViewController:hrViewController];
    
	[deviceWindow makeKeyAndVisible];
        
    [UIApplication sharedApplication].idleTimerDisabled = YES;
	 
	externalWindow = [[UIWindow alloc] initWithFrame:CGRectZero];
	externalWindow.hidden = YES;
	 	
	if(g_pref_nativeTVOUT)
	{ 	
		[[NSNotificationCenter defaultCenter] addObserver:self 
													 selector:@selector(prepareScreen) 
														 name:/*@"UIScreenDidConnectNotification"*/UIScreenDidConnectNotification
													   object:nil];
	        
			
	    [[NSNotificationCenter defaultCenter] addObserver:self 
													 selector:@selector(prepareScreen) 
														 name:/*@"UIScreenDidDisconnectNotification"*/UIScreenDidDisconnectNotification
													   object:nil];
	}	
    
    [self prepareScreen];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {

   if((myosd_inGame || g_joy_used ) && !isGridlee )//force pause when game
      [hrViewController runMenu];
}

- (void)applicationWillResignActive:(UIApplication *)application {
    
#ifdef BTJOY
    [BTJoyHelper endBTJoy];
#endif
}

/*
- (void)applicationDidBecomeActive:(UIApplication  *)application {
  
}

- (void)applicationWillTerminate:(UIApplication *)application {
 
}
*/

- (void)prepareScreen
{
	 @try
    {												   										       
	    if ([[UIScreen screens] count] > 1 && g_pref_nativeTVOUT) {
	    											 	        	   					
			// Internal display is 0, external is 1.
			externalScreen = [[UIScreen screens] objectAtIndex:1];			
			screenModes =  [externalScreen availableModes];
					
			// Allow user to choose from available screen-modes (pixel-sizes).
			UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"External Display Detected!" 
															 message:@"Choose a size for the external display." 
															delegate:self 
												   cancelButtonTitle:nil 
												   otherButtonTitles:nil];
			for (UIScreenMode *mode in screenModes) {
				CGSize modeScreenSize = mode.size;
				[alert addButtonWithTitle:[NSString stringWithFormat:@"%.0f x %.0f pixels", modeScreenSize.width, modeScreenSize.height]];
			}
			[alert show];
			
		} else {
		     if(!g_emulation_initiated)
		     {
		        [hrViewController startEmulation];
		     }   
		     else
		     {
		        [hrViewController setExternalView:nil];
		        externalWindow.hidden = YES;
		        [hrViewController changeUI];
		     }   
		    	
		}
	}
	 @catch(NSException* ex)
    {
        NSLog(@"Not supported tv out API!");
        if(!g_emulation_initiated)
          [hrViewController startEmulation];
    }	
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	
	[externalScreen setCurrentMode:[screenModes objectAtIndex:buttonIndex]];
	[externalWindow setScreen:externalScreen];
	
	CGRect rect = CGRectZero;
 	
	rect = externalScreen.bounds;
	externalWindow.frame = rect;
	externalWindow.clipsToBounds = YES;
	
	int  external_width = externalWindow.frame.size.width;
	int  external_height = externalWindow.frame.size.height;
	
	float overscan = 1 - (g_pref_overscanTVOUT *  0.025f);

    int width=external_width;
    int height=external_height; 

    width = width * overscan;    
    height = height * overscan;
    int x = (external_width - width)/2;
    int y = (external_height - height)/2;
                                       
    CGRect rView = CGRectMake( x, y, width, height);
    
    for (UIView *view in [externalWindow subviews]) {
       [view removeFromSuperview];
    }
    
    UIView *view= [[UIView alloc] initWithFrame:rect];
    view.backgroundColor = [UIColor blackColor];
    [externalWindow addSubview:view];
		
    [hrViewController setExternalView:view];
    hrViewController.rExternalView = rView;
    
	externalWindow.hidden = NO;
	//[externalWindow makeKeyAndVisible];
	if(g_emulation_initiated)
	    [hrViewController changeUI];
	else
	    [hrViewController startEmulation];
	    
}



@end