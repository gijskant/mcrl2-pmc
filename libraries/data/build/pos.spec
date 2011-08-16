% Author(s): Aad Mathijssen, Jeroen Keiren
% Copyright: see the accompanying file COPYING or copy at
% https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
%
% Distributed under the Boost Software License, Version 1.0.
% (See accompanying file LICENSE_1_0.txt or copy at
% http://www.boost.org/LICENSE_1_0.txt)
%
% Specification of the Pos data sort.

#include bool.spec

sort Pos <"pos">;

cons @c1 <"c1">:Pos;
     @cDub <"cdub">:Bool <"bit"> #Pos <"number"> ->Pos;

map max <"maximum">:Pos <"left"> #Pos <"right">->Pos;
    min <"minimum">:Pos <"left"> #Pos <"right">->Pos;
    succ <"succ">:Pos <"number">->Pos;
    + <"plus">:Pos <"left"> #Pos <"right">->Pos;
    @addc <"add_with_carry">:Bool <"bit"> #Pos <"left"> #Pos <"right">->Pos;
    * <"times">:Pos <"left"> #Pos <"right">->Pos;

var b:Bool;
    c:Bool;
    p:Pos;
    q:Pos;
    r:Pos;
eqn ==(@c1, @cDub(b,p)) = false;
    ==(@cDub(b,p), @c1) = false;
    ==(@cDub(false,p), @cDub(true,q)) = false;
    ==(@cDub(true,p), @cDub(false,q)) = false;
    ==(@cDub(b,p), @cDub(b, q)) = ==(p,q);
    ==(@cDub(b,p), @cDub(c, q)) = &&(==(b,c), ==(p,q));
    <(p, @c1) = false;
    <(@c1, @cDub(b,p)) = true;
    <(@cDub(b,p), @cDub(b,q)) = <(p,q);
    <(@cDub(false,p), @cDub(true, q)) = <=(p,q);
    <(@cDub(b,p), @cDub(false, q)) = <(p,q);
% The equation below is required for the enumeration of lists
    <(@cDub(b,p), @cDub(c,q)) = if(=>(c,b), <(p,q), <=(p,q));
    <=(@c1, p) = true;
    <=(@cDub(b,p), @c1) = false;
    <=(@cDub(b,p), @cDub(b, q)) = <=(p,q);
    <=(@cDub(false,p), @cDub(b,q)) = <=(p,q);
    <=(@cDub(true,p), @cDub(false, q)) = <(p,q);
% The equation below is required for the enumeration of lists
    <=(@cDub(b,p), @cDub(c,q)) = if(=>(b,c), <=(p,q), <(p,q));
    max(p,q) = if(<=(p,q),q,p);
    min(p,q) = if(<=(p,q),p,q);
    succ(@c1) = @cDub(false, @c1);
    succ(@cDub(false,p)) = @cDub(true,p);
    succ(@cDub(true,p)) = @cDub(false,succ(p));
    +(p,q) = @addc(false,p,q);
    @addc(false,@c1,p) = succ(p);
    @addc(true,@c1,p) = succ(succ(p));
    @addc(false,p,@c1) = succ(p);
    @addc(true,p,@c1) = succ(succ(p));
    @addc(b,@cDub(c,p),@cDub(c,q)) = @cDub(b,@addc(c,p,q));
    @addc(b,@cDub(false,p),@cDub(true,q)) = @cDub(!(b), @addc(b,p,q));
    @addc(b,@cDub(true,p),@cDub(false,q)) = @cDub(!(b), @addc(b,p,q));
    *(@c1,p) = p;
    *(p,@c1) = p;
    *(@cDub(false,p),q) = @cDub(false,*(p,q));
    *(p,@cDub(false,q)) = @cDub(false,*(p,q));
    *(@cDub(true,p),@cDub(true,q)) = @cDub(true,@addc(false,p,@addc(false,q,@cDub(false,*(p,q)))));
