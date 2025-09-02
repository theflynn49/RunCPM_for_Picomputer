program hello;
var
  I,J: Integer;
begin
  writeln(#12'Hello '#$1b'[94m'#$1b'[40mWorld! ');
  I:=0;
  while I<8 do
  begin
    write(#$1B'[');
    write(I+30);
    write('m00 ');
    write(#$1B'[');
    write(I+90);
    write('m00 ');
    I:=I+1;
  end;
   writeln;
   I:=0;J:=0;
   while I<256 do
   begin
      write(#$1B'[38;5;');
      write(I);
      write('m0');
      J:=J+1;I:=I+1;

      if J=36 then begin
        J:=0;
        writeln;
      end;
   end;
   writeln(#$1B'[1m');
  I:=0;
  while I<8 do
  begin
    write(#$1B'[');
    write(I+40);
    write('m ');
    write(#$1B'[');
    write(I+100);
    write('m ');
    I:=I+1;
  end;
   writeln;
   I:=0;J:=0;
   while I<256 do
   begin
      write(#$1B'[48;5;');
      write(I);
      write('m1');
      J:=J+1;I:=I+1;

      if J=36 then begin
        J:=0;
        writeln;
      end;
   end;
   writeln(#$1B'[1m');
end.

