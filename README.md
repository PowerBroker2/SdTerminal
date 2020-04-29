# SdTerminal
Basic terminal interface for Arduino SD card logging/file management. Allows basic file management of an SD card with an Arduino through serial commands.

## Commands:

`ls` -- List all files on SD card <br/>
`rm all` -- Delete all files on SD card except for the file currently being used for datalogging <br/>
`rm <filename>` -- Delete the specified file on the SD card except for the file currently being used for datalogging <br/>
`<filename>` -- Display the entire contents of the file

## Notes:

The returned message `?` signifies the last command was invalid
