unit mikan;

interface
  procedure syscall;

implementation

{$IFDEF MIKAN_BARE}
  {$I mikan_bare.inc}
{$ELSE}
  {$I mikan_hosted.inc}
{$ENDIF}

end.
