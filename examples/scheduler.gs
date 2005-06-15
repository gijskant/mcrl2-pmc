% This is a specification of Milner's well known scheduler
% that can already be found in Milner's famous Lecture Note in Computer Science
% 92, A Calculus of Communicating Systems, Springer, 1980.

act a,b,t,tcomm:Nat;

proc Cycler(i:Nat,n:Pos) = 
           t(i).a(i).
             (t((i+1) mod n).b(i)+
              b(i).t((i+1) mod n)).Cycler(i,n);

     Start = t(0).delta;

     Scheduler2=allow({a,b,tcomm},comm({t|t->tcomm},Start||Cycler(0,2)||Cycler(1,2)));
     Scheduler3=allow({a,b,tcomm},comm({t|t->tcomm},Start||Cycler(0,3)||Cycler(1,3)||Cycler(2,3)));
     Scheduler4=allow({a,b,tcomm},comm({t|t->tcomm},Start||Cycler(0,4)||Cycler(1,4)||Cycler(2,4)||
                                     Cycler(3,4)));
     Scheduler5=allow({a,b,tcomm},comm({t|t->tcomm},Start||Cycler(0,5)||Cycler(1,5)||Cycler(2,5)||
                                     Cycler(3,5)||Cycler(4,5)));
     Scheduler6=allow({a,b,tcomm},comm({t|t->tcomm},Start||Cycler(0,6)||Cycler(1,6)||Cycler(2,6)||
                                     Cycler(3,6)||Cycler(4,6)||Cycler(5,6)));
                
init hide({tcomm},Scheduler2);
