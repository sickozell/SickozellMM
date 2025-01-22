# SickoCVexp v2.0.0-beta
VCV Rack plugin modules

## randLoops
### looped random voltage sequencer inspired by Turing Machine

![randloops](https://github.com/user-attachments/assets/2a585c43-5cd4-44df-93bc-55e89d954cdf)

#### - INSTRUCTIONS
randLoops replicates functionalities of Turing Machine module by Tom Whitwell, please refer to several instruction/tutorials on the web, for main use.  
There are some added features that can be helpful using it with VCV rack.  
ADD and DEL buttons or a trigger on their inputs, just like the 'write' switch, will overwrite the first incoming bit forcing it to be on or off.  
RND button or a trigger on its input generates a new sequence.  
A trigger on RST input clears the sequence.  
Either ADD/DEL controls or RND button/trig are buffered by default, it means that changes will have effect only when a clock pulse is detected. These options can be modified in the right-click menu.

[back to top](#table-of-contents)