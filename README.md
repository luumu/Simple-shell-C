Simple shell written in C

The shell can 

1.execute a command with its arguments (ls -l -a -S , wc -l -m, pwd, cat...etc)

2.move between directories with cd command (cd somedir, cd ..) and go to default home directory if cd is entered without arguments

3.Display current working directory in prompt

4.redirect IO (md5sum < somefile > someotherfile etc)

5.run multiple piped commands with or without io redirection so that commands such as 

ls -l | wc -l,

md5sum < somefile | wc -m > someotherfile,  

ls -l -a | head -5 | tail -2 | wc -l,

ls -l -a | head -5 | tail -2 | wc -m > somefile, etc work.