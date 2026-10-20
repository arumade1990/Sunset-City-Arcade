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

**10/18/26** 
The entire source code version is now ready to do a full scale hardware test as of the latest commit. Will test out at a later date.

**10/19/26**
Tested code on physical hardware. After some debugging, the features tested worked well. 

Main issues encountered:
----MC2 Not shutting off properly in shutdown sequence. This was caused by a quirk in the TimerON class where it toggles the output instead of taking it to a specific state. I may change this behaviour in that class in the future, but for now I'm leaving it since it's fairly easy to work around.

----In the same function, the Total Power Off output was not coming onat all. This was due to a logic flaw that I had in an earlier part of the function locking out the statment that turns the output on. Once identified, I was able to do a slight re-work of the logic and got it working smoothly. 

As of right now there are a few things that I need to test before putting this whole setup together with the arcade PC rig. These items include the rest of the maintenance fucntions, the faults functions and the fault reset functions. The stepper controller jog functions also need to be fully tested if possible too, but I'm not sure of a good way to do that without just hooking it up the the whole PC. Either way it should be OK. 

**10/20/26**
Did more tests on the hardawre setup. Very pleased with the results. I probably need to test the fault system a bit more but honestly with how things act/run right now it should be totally fine as is. Maybe some minor tweaks in the future, but the bulk of this feels done. Yay! The sequences feel snappy, the delays feel purposeful for safe, controlled startup (I think...), and overall the thing that will take the longest in the whole power up sequence will be waiting on the PC to boot up and start running the emulation software. 

I was also able to devise a way to test the jog function of the stepper controller and it works beautifully, which is great because I wrote it kinda blind and without doing much preliminary or exploratory testing. I'm SURE there is something I'm missing but I believe that my goals for this part of the build have largely been met and overall are going in the right direction. 

Things left to do before doing the full test run with the total rig:
----Solder up the revised stepper controller/FT232H board, and MUCH needed upgrade, but I previously did not want to chagne the test conditions since I knew my botched one worked after I fixed my mistakes on it. The new one should be much more reliable in the long run. 

----Button up the wiring and update the prints. As you might expect the wiring has gotten a bit out of sorts during the weeks of testing. It needs to be tidied up again and put back in a state to be ready to install into a cabinet! (Finally!) Also there are some chagnes to the wiring that I didn't knwo would happen until I started putting things through their paces so that should really be documented for future reference. Theres over 250 feet of 16awg wire in this damn thing and even knowing how it's build inside and out it can be tough without at least a reminder of what goes where!

----Adjust the load wheels on the stepper mount. One side doesn't touch fully and it needs to. Should e a fairly easy adjustment but definitely easier with the screen not being on there. 

All in all, this is an exciting time for this project! I've been woring on this for so long and finally its starting to shape up nicely!