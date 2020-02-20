# HatJumpCalc

## Boredom-fueled Introduction

This code is released because, peck, why not make it open source when there might be someone who can make something out of this pecked up code? This code really is filled with whatever I wanted to do with it, and I am shipping it *without* removing dirty, hacky, dead, or otherwise questionable parts. This is a fine specimen of a code that is written by a person whose major is English (btw it's non-anglophone uni) for personal purposes, fresh out of VSCode!

You'll need to tweak some of the defines and constants to make use of this. The code was only tested on MINGW, but I am sure that a few linux-savvy people can make this run it in linux since that wouldn't be too much hassle, I guess? (It *might* run on linux out-of-the-box, so give it a try! i use gentoo too btw)

I also added some "Masterfiles" (a.k.a. _extremely_ uselessly large file filled with results) for those who somehow can't run it; the format is questionable at best, but, hey, I am releasing it at 4 AM in the middle of mild depression and bad health. The distances at the "Masterfiles" are "gaps" between two ledges, both perpendicular to the Hat Kid's trajectory. You can take a look at my map to "feel" what this means: https://steamcommunity.com/sharedfiles/filedetails/?id=1976610711

Files Trimmed at -10000 HeightDiff are also available. I recommend downloading trimmed ones, for that is much more sanity- and eco-friendly. Think of the CPU!

Since Excel can't handle that much precision, the provided "Masterfiles" has much less precision than fresh data. If you really want, you can get those versions by making it yourself. I gave you the sauce!

## Glossary for the Masterfiles for dummies (all of these can be learnt from code):

* CJ = CommonJump; jump, another jump, dash, dash cancel
* SJ = ScooterJump; hop on dat pekken scooter, jump, another jump
* HeightDiff = Height difference between the starting platform and the destination platform. Lower than -2000 would be impractical, but I calculated way, way, WAY too far down, to the point of irredeemable uselessness.
* Gracew = That short period of time where you are definitely falling but magically(it is intentional, and it's actually 0.2 seconds) able do normal jump instead of double jump
* Fw = First wait; ticks between first jump and second jump
* Sw = Second wait; ticks between second jump and dash (only for CJ)
* Dw = Dash wait; ticks between dash and dash cancel
* Frame = It magically means the whole airtime in ticks
* PosZ = Z Pos where the airtime ends. Absolutely useless but for the sake of the DATA!
* PosX = THE ACTUAL RESULT; the maximum achievable "gap" between two ledges on that HeightDiff with CJ of SJ. (rule of thumb about humanly possible maximum is 2% less than this.) If you want to use this in editor, you should also see the object's dimension. Good thing is that you can do some basic arithmetics in objects' position fields in A Hat in Time Editor.