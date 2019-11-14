procedure syscall;
begin
  asm
    svc #0
  end;
end;
