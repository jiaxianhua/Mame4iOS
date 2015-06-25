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

#import "InputOptionController.h"
#import "Globals.h"
#import "OptionsController.h"
#import "ListOptionController.h"
#import "EmulatorController.h"

#include "myosd.h"

@implementation InputOptionController

@synthesize emuController;

- (id)init {
    if (self = [super initWithStyle:UITableViewStyleGrouped]) {
        
        switchAnimatedButtons=nil;

        arrayTouchType = [[NSArray alloc] initWithObjects:@"Digital DPAD",@"Digital Stick",@"Analog Stick", nil];
        arrayStickType = [[NSArray alloc] initWithObjects:@"Auto",@"2-Way",@"4-Way",@"8-Way", nil];
        arrayStickSizeValue = [[NSArray alloc] initWithObjects:@"Smaller", @"Small", @"Normal", @"Big", @"Bigger",nil];
        
        arrayNumbuttons = [[NSArray alloc] initWithObjects:@"Auto",@"0 Buttons",@"1 Buttons",@"2 Buttons",@"3 Buttons",@"4 Buttons",@"All Buttons", nil];
        switchAplusB = nil;
        arrayAutofireValue = [[NSArray alloc] initWithObjects:@"Disabled", @"Speed 1", @"Speed 2",@"Speed 3",
                              @"Speed 4", @"Speed 5",@"Speed 6",@"Speed 7",@"Speed 8",@"Speed 9",nil];
        arrayButtonSizeValue = [[NSArray alloc] initWithObjects:@"Smaller", @"Small", @"Normal", @"Big", @"Bigger",nil];
        
        arrayControlType = [[NSArray alloc] initWithObjects:@"None",@"iCade",@"iCP, Gametel",@"iMpulse", nil];
        
        switchP1aspx = nil;
        
        switchTouchDeadZone = nil;
        arrayAnalogDZValue = [[NSArray alloc] initWithObjects:@"1", @"2", @"3",@"4", @"5", @"6", nil];
        arrayBTDZValue = [[NSArray alloc] initWithObjects:@"1", @"2", @"3",@"4", @"5", @"6", nil];
        
        self.title = @"Input Options";
    }
    return self;
}


- (void) viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    UITableView *tableView = (UITableView *)self.view;
    [tableView reloadData];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    
    return 7;
}

- (void)loadView {
    
    [super loadView];
    
    UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithTitle:@"Done"
                                                               style:UIBarButtonItemStyleBordered
                                                              target: emuController  action:  @selector(done:) ];
    self.navigationItem.rightBarButtonItem = button;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    switch (section)
    {
        case 0: return 1;
        case 1: return 3;
        case 2: return 4;
        case 3: return 1;
        case 4: return 2;
        case 5: return 1;
        case 6: return 3-!g_btjoy_available;
    }
    return -1;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    
    switch (section)
    {
        case 0: return @"";
        case 1: return @"Stick & DPAD";
        case 2: return @"Buttons";
        case 3: return @"";
        case 4: return @"Touch Layout";
        case 5: return @"";
        case 6: return @"Dead Zone";
    }
    return @"Error!";
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    NSString *cellIdentifier = [NSString stringWithFormat: @"%d:%d", [indexPath indexAtPosition:0], [indexPath indexAtPosition:1]];
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if (cell == nil)
    {
        
        UITableViewCellStyle style;
        
        if(indexPath.section==4)
            style = UITableViewCellStyleDefault;
        else
            style = UITableViewCellStyleValue1;
        
        cell = [[UITableViewCell alloc] initWithStyle:style
                                       reuseIdentifier:@"CellIdentifier"];
        
        cell.accessoryType = UITableViewCellAccessoryNone;
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    
    Options *op = [[Options alloc] init];
    
    switch (indexPath.section)
    {
        case 0:
        {
            cell.textLabel.text   = @"Animated";
            switchAnimatedButtons  = [[UISwitch alloc] initWithFrame:CGRectZero];
            cell.accessoryView = switchAnimatedButtons ;
            [switchAnimatedButtons setOn:[op animatedButtons] animated:NO];
            [switchAnimatedButtons addTarget:self action:@selector(optionChanged:) forControlEvents:UIControlEventValueChanged];
            break;
        }
        case 1:
        {
            switch (indexPath.row)
            {
                case 0:
                {
                    cell.textLabel.text   = @"Touch Type";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayTouchType objectAtIndex:op.touchtype];
                    break;
                }
                    
                case 1:
                {
                    cell.textLabel.text   = @"Ways";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayStickType objectAtIndex:op.sticktype];
                    break;
                }
                    
                case 2:
                {
                    cell.textLabel.text   = @"Fullscreen Stick Size";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayStickSizeValue objectAtIndex:op.stickSize];
                    break;
                }
            }
            break;
        }
        case 2:
        {
            switch (indexPath.row)
            {   case 0:
                {
                    cell.textLabel.text   = @"Fullscreen Buttons";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayNumbuttons objectAtIndex:op.numbuttons];
                    break;
                }
                    
                case 1:
                {
                    cell.textLabel.text   = @"Button A = B + X";
                    switchAplusB  = [[UISwitch alloc] initWithFrame:CGRectZero];
                    cell.accessoryView = switchAplusB ;
                    [switchAplusB setOn:[op aplusb] animated:NO];
                    [switchAplusB addTarget:self action:@selector(optionChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 2:
                {
                    
                    cell.textLabel.text   = @"Button B as Autofire";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayAutofireValue objectAtIndex:op.autofire];
                    break;
                }
                case 3:
                {
                    cell.textLabel.text   = @"Buttons Size";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayButtonSizeValue objectAtIndex:op.buttonSize];
                    break;
                }
            }
            break;
        }
        case 3:
        {
            cell.textLabel.text   = @"External Controller";
            cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
            cell.detailTextLabel.text = [arrayControlType objectAtIndex:op.controltype];
            break;
        }
        case 4:
        {
            switch (indexPath.row)
            {   case 0:
                {
                    cell.selectionStyle = UITableViewCellSelectionStyleBlue;
                    cell.textLabel.text = @"Change Current Layout";
                    cell.textLabel.textAlignment = UITextAlignmentCenter;
                    break;
                }
                case 1:
                {
                    cell.selectionStyle = UITableViewCellSelectionStyleBlue;
                    cell.textLabel.text = @"Reset Current Layout to Default";
                    cell.textLabel.textAlignment = UITextAlignmentCenter;
                    break;
                }
            }
            break;
        }
        case 5:
        {
            cell.textLabel.text   = @"P1 as P2,P3,P4";
            switchP1aspx  = [[UISwitch alloc] initWithFrame:CGRectZero];
            cell.accessoryView = switchP1aspx ;
            [switchP1aspx setOn:[op p1aspx] animated:NO];
            [switchP1aspx addTarget:self action:@selector(optionChanged:) forControlEvents:UIControlEventValueChanged];
            break;
        }
        case 6:
        {
            switch (indexPath.row)
            {   case 0:
                {
                    cell.textLabel.text   = @"Touch DPAD";
                    switchTouchDeadZone  = [[UISwitch alloc] initWithFrame:CGRectZero];
                    cell.accessoryView = switchTouchDeadZone ;
                    [switchTouchDeadZone setOn:[op touchDeadZone] animated:NO];
                    [switchTouchDeadZone addTarget:self action:@selector(optionChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 1:
                {
                    cell.textLabel.text   = @"Touch Stick";
                    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                    cell.detailTextLabel.text = [arrayAnalogDZValue objectAtIndex:op.analogDeadZoneValue];
                    break;
                }
                case 2:
                {
                    if(g_btjoy_available)
                    {
                        cell.textLabel.text   = @"BT Analog";
                        
                        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
                        cell.detailTextLabel.text = [arrayBTDZValue objectAtIndex:op.btDeadZoneValue];
                    }
                    break;
                }
            }
            break;
        }
    }
    
    
    return cell;
}

- (void)optionChanged:(id)sender
{
    Options *op = [[Options alloc] init];
    
	if(sender == switchAnimatedButtons)
        op.animatedButtons=  [switchAnimatedButtons isOn];
	if(sender == switchTouchDeadZone)
        op.touchDeadZone = [switchTouchDeadZone isOn];
    if(sender == switchAplusB)
        op.aplusb = [switchAplusB isOn];
    if(sender == switchP1aspx)
        op.p1aspx = [switchP1aspx isOn];
    
    [op saveOptions];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
    NSUInteger row = [indexPath row];
    NSUInteger section = [indexPath section];
    
    if(section==1 && row==0)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped type:kTypeTouchType list:arrayTouchType];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    if(section==1 && row==1)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                    type:kTypeStickType list:arrayStickType];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    if(section==1 && row==2)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeStickSizeValue list:arrayStickSizeValue];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    
    
    if(section==2 && row==0)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeNumButtons list:arrayNumbuttons];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    if(section==2 && row==2)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeAutofireValue list:arrayAutofireValue];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    
    if(section==2 && row==3)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeButtonSizeValue list:arrayButtonSizeValue];
        [[self navigationController] pushViewController:listController animated:YES];
    }

    if(section==3 && row==0)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeControlType list:arrayControlType];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    
    if(section==4 && row==0)
    {
        [emuController beginCustomizeCurrentLayout];
        [tableView reloadData];
    }
    if(section==4 && row==1)
    {
        [emuController resetCurrentLayout];
        [tableView reloadData];
    }

    if(section==6 && row==1)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeAnalogDZValue list:arrayAnalogDZValue];
        [[self navigationController] pushViewController:listController animated:YES];
    }
    if(section==6 && row==2)
    {
        ListOptionController *listController = [[ListOptionController alloc] initWithStyle:UITableViewStyleGrouped
                                                                                      type:kTypeBTDZValue list:arrayBTDZValue];
        [[self navigationController] pushViewController:listController animated:YES];
    }

    
}
    

@end
