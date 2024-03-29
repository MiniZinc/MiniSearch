%-----------------------------------------------------------------------------%
% Radiation problem, Minizinc version
%
% Sebastian Brand
% 05/2007
%-----------------------------------------------------------------------------%
include "minisearch.mzn";

%-----------------------------------------------------------------------------%
% Parameters
%-----------------------------------------------------------------------------%

int: m;  % Rows
int: n;  % Columns

set of int: Rows    = 1..m;
set of int: Columns = 1..n;

	% Intensity matrix
array[Rows, Columns] of int: Intensity;


set of int: BTimes = 1..Bt_max;

int: Bt_max   = max(i in Rows, j in Columns) (Intensity[i,j]);
int: Ints_sum = sum(i in Rows, j in Columns) (Intensity[i,j]);

%-----------------------------------------------------------------------------%
% Variables
%-----------------------------------------------------------------------------%

	% Total beam-on time
var 0..Ints_sum: Beamtime;

	% Number of shape matrices
var 0..m*n: K;

	% N[b] is the number of shape matrices with associated beam-on time b
array[BTimes] of var 0..m*n: N;

	% Q[i,j,b] is the number of shape matrices with associated beam-on time
	% b that expose cell (i,j)
array[Rows, Columns, BTimes] of var 0..m*n: Q;

%-----------------------------------------------------------------------------%
% Constraints
%-----------------------------------------------------------------------------%

	% For FD/LP hybrid solving, all these should go the LP solver
	% (with a suitable linearisation of the 'max' expressions).
constraint
	Beamtime = sum(b in BTimes) (b * N[b])
	/\
	K = sum(b in BTimes) (N[b])
	/\
	forall(i in Rows, j in Columns)
		( Intensity[i,j] = sum([b * Q[i,j,b] | b in BTimes]) )
	/\
	forall(i in Rows, b in BTimes)
		( upper_bound_on_increments(N[b], [Q[i,j,b] | j in Columns]) );


predicate upper_bound_on_increments(var int: N_b, array[int] of var int: L) =
	N_b >= L[1] + sum([ max(L[j] - L[j-1], 0) | j in 2..n ]);
	%
	% Good linear version:
	% let { array[min(index_set(L))..max(index_set(L))] of var int: S } in
	% N_b >= L[1] + sum([ S[j] | j in 2..n ]) /\
	% forall([ S[j] >= L[j] - L[j-1] /\ S[j] >= 0 | j in 2..n ]);
	

%-----------------------------------------------------------------------------%
% Objective
%-----------------------------------------------------------------------------%
var int: obj;
constraint obj = (ub(K) + 1) * Beamtime + K;

int: timeout_ms = 10*1000;

solve 
    :: int_search(
		[Beamtime] ++ N ++
	    [Q[i,j,b] | i in Rows, j in Columns, b in BTimes ],
		input_order, indomain_split, complete)
search 
    time_limit(timeout_ms, minimize_bab(obj)) /\ 
    if hasSol() then print() else print("No solution found within timelimit.\n") endif;
%	minimize obj;
		% really: (Beamtime, K),  i.e. in lexicographic order

%-----------------------------------------------------------------------------%
% Instance 
%-----------------------------------------------------------------------------%
m = 5;	% rows
n = 5;	% columns

% max intensity = 8
Intensity = [|
6, 4, 5, 3, 4 |
2, 3, 0, 1, 8 |
0, 7, 5, 6, 3 |
5, 2, 3, 1, 2 |
0, 1, 8, 0, 6
|];



output [
    show(obj)
];
%    "Beamtime = ", show(Beamtime), "\n",
%    "K = ", show(K), "\n",
%    "N = ", show(N), "\n",
%    "Q = ", show(Q), "\n",

%-----------------------------------------------------------------------------%
%-----------------------------------------------------------------------------%
