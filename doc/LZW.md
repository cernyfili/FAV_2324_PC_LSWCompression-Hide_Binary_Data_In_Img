# LZW Compression

```{r,
    Initialize table with single character strings

     P_INPUT_CHAR = first input character
   
     WHILE not end of input stream
   
          C_NEXT_CHAR = next input character
       
          IF P_INPUT_CHAR + C_NEXT_CHAR is in the string table
        
            P_INPUT_CHAR = P_INPUT_CHAR + C_NEXT_CHAR
          
          ELSE
        
            output the code for P_INPUT_CHAR
          
            add P_INPUT_CHAR + C_NEXT_CHAR to the string table
        
            P_INPUT_CHAR = C_NEXT_CHAR
        
     END WHILE
   
    output code for P_INPUT_CHAR
```

# LZW Decompression

```{r,
    Initialize table with single character strings
  
    OLD_code = first input code
    save to result translation of OLD
  
    WHILE not end of input stream
        NEW_code = next input code
        IF NEW_code is not in the string table
               S = translation of OLD_code
               S = S + C
       ELSE
              S = translation of NEW
       output S
       C = first character of S
       OLD_value + C to the string table
       OLD_code = NEW_code
   END WHILE
```

# Links

## LZW with trie

[https://github.com/albertnadal/lzw-encoder](https://github.com/albertnadal/lzw-encoder)

## LZW other

[https://github.com/MichaelDipperstein/lzw/blob/master/lzwdecode.c](https://github.com/MichaelDipperstein/lzw/blob/master/lzwdecode.c)
