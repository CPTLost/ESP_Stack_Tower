# ESP_TOWER_STACK THE GAME

This Project was developed for the "Development Board V1.2 zum Buch Embedded Systems mit RISC-V und ESP32C-3" in combination with a 128/64 OLED Screen.

The Idea behind this game is simple. The goal is to stack blocks on top of each other as precise as possible.
Over time the speed with which the blocks travel over the screen increases, which makes it more difficult to stack them.
When you mispalce a block it will be cut of where the block below ends. The next appearing block will have the same size as the top block of the current tower. When you misplace a block completely or miss to place it in the first place you will lose the game and your honour will be deminished for **eternity**. 

I hope you have fun with it :)


## Idea behind the Code

To realise this game I had to answer 2 main questions. 

1. how can a block be animated smoothly?
2. how can I keep track of the stacking process?

The answer to the first question is: a loop in which each iteration displays one frame. With the proper delay between iterations we can achieve different animation speeds, which comes handy when trying to set a specific difficulty.

The answer to the second question is: I defined a block with its width and height. This information is saved in a struct called block. Now that I have defined a block I can determine its position on the screen by only knowing its y coordinate, the rest can be calculated with the height and width. 

When I now combine those two concepts I need something that stops the animation. This calls for an interrupt. So I used an interrupt which is triggered by a button press to stop the animation and save its y coordinate. For the next loop I increment the position of the block animation (level) so that it will be animated above the block before. When now the interrupt is triggered I can calculate the overhang of the placed blocks and check if its completely or only partly off. This overhang is then subtracted from the block width, so the next block is smaller. This goes on until the block is gone, or it ran out of bounds during the animation.
Therefore I used one Main while loop to loop through every level and then inside it there is another loop for animating the block on the current level.

I hope this basic summary helps to understand the code a bit better.
