This repository is a central place for me to upload my code for my sunset City Arcade project. 

For those unaware, this is a project that I've not seen anywhere and hopefully will be a really fun end result. 

Additionally, please pardon my coding, I am 100% self taught and there are likely some weird thins going on simply because I don't know any better, however some thing I do know are kind of weird but i want them that way specifically. I'll leave it up to you to figure out which one it which!

Anyhow, this project's goal is to create a system that is the following:

1) Very kid friendly, and difficult to get into a bad state or power down/up the wrong way.
2) Easily serviceable with dedicated maintenance features and modes.
3) Reliable and resilient like the beloved arcade cabs of yester year. Something that can take a beating and just keep going. (This is more on the hardware side which I am vastly more comfortable with.)
4) Be fully automated. As in, if you select a vertical screen game, the system detects that and turns the screen to the right position AND back when exiting the game.
5) Able to be turned off at basically any state and the system will put itself into a good 'Home Position' for when it's booted up the next time.

Now of course, there are variables out of my control, like power outages and such, but that it what the maintenance modes are for, so you can re-home the screen and get things back to normal relatively easy. 

Thanks for coming to my TED talk. 
-Andy

**=====UPDATES====**
10/18/26 
The entire source code version is now ready to do a full scale hardware test as of the latest commit. Will test out at a later date.

10/19/26
Tested code on physical hardware. After some debugging, the features tested worked well. 
Main issues encountered:
-MC2 Not shutting off properly in shutdown sequence. This was caused by a quirk in the TimerON class where it toggles the output instead of taking it to a specific state. I may change this behaviour in that class in the future, but for now I'm leaving it since it's fairly easy to work around.
-In the same function, the Total Power Off output was not coming onat all. This was due to a logic flaw that I had in an earlier part of the function locking out the statment that turns the output on. Once identified, I was able to do a slight re-work of the logic and got it working smoothly. 

As of right now there are a few things that I need to test before putting this whole setup together with the arcade PC rig. These items include the rest of the maintenance fucntions, the faults functions and the fault reset functions. The stepper controller jog functions also need to be fully tested if possible too, but I'm not sure of a good way to do that without just hooking it up the the whole PC. Either way it should be OK. 
