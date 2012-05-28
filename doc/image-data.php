<?php
file_put_contents("email_wave.txt", "data:image/jpeg;base64,".base64_encode(file_get_contents("./email_wave.jpg")));
file_put_contents("email_logo.txt", "data:image/png;base64,".base64_encode(file_get_contents("./email_logo.png")));
