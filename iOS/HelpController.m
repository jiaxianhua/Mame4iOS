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

#import "HelpController.h"
#import "Globals.h"

@implementation HelpController


- (id)init {

    if (self = [super init]) {
        aWebView  = nil;
    }

    return self;
}

- (void)loadView {
       
	UIView *view= [[UIView alloc] initWithFrame:[[UIScreen mainScreen] applicationFrame]];
	self.view = view;
    
    self.title = @"Help";
    self.view.backgroundColor = [UIColor whiteColor];
    self.view.autoresizesSubviews = TRUE;
        
    aWebView =[ [ UIWebView alloc ] initWithFrame: view.frame];
    
    //aWebView.scalesPageToFit = YES;
    
    aWebView.autoresizingMask = (UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight);
    
    [ self.view addSubview: aWebView ];
}

-(void)viewDidLoad{	

}

-(void)viewWillAppear:(BOOL)animated
{
    aWebView.delegate = self;
    
    [self loadHTML];
}

-(void)viewWillDisappear:(BOOL)animated
{
    [aWebView stopLoading];
    aWebView.delegate = nil;
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    //   return (interfaceOrientation == UIInterfaceOrientationPortrait);
    return YES;
    //return NO;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    
    //[self loadHTML];
    //[aWebView reload];    
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
}

 
- (void)dealloc {
    
    aWebView.delegate = nil;

}

-(BOOL) webView:(UIWebView *)inWeb shouldStartLoadWithRequest:(NSURLRequest *)inRequest navigationType:(UIWebViewNavigationType)inType {
    if ( inType == UIWebViewNavigationTypeLinkClicked ) {
        [[UIApplication sharedApplication] openURL:[inRequest URL]];
        return NO;
    }
    
    return YES;
}

- (void)loadHTML{
    NSString *HTMLData = [[NSString alloc] initWithContentsOfFile: [NSString stringWithUTF8String:get_resource_path("help.html")] encoding:NSUTF8StringEncoding error:nil];
    
    NSURL *aURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:get_resource_path("")]];
    
    [aWebView loadHTMLString:HTMLData baseURL: aURL];
    
}

@end