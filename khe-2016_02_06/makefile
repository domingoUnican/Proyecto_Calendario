############################################################################
#                                                                          #
#  Make file for compiling KHE                                             #
#                                                                          #
#  Jeffrey H. Kingston                                                     #
#                                                                          #
#     make              Compile KHE                                        #
#     make khe          Ditto                                              #
#     make clean        Undo "make khe".                                   #
#                                                                          #
#  Although "make khe" produces binary "khe", which invokes the main()     #
#  function in khe_main.c, you can throw away that binary, throw away      #
#  file khe_main.o, and link the other .o files to your own code.  You     #
#  lose nothing by doing that except main().                               #
#                                                                          #
#  KHE uses the pthread system library.  If you don't have that library,   #
#  you can still compile KHE, by doing this:                               #
#                                                                          #
#  (1) Remove "-pthread" from the CFLAGS line just below.                  #
#  (2) Remove "-pthread" from another point further down.                  #
#  (3) Find line "#define KHE_USE_PTHREAD 1" near the top of file khe.h    #
#      and change the 1 to a 0.                                            #
#                                                                          #
#  The only difference will be that function KheParallelSolve() will       #
#  solve its multiple solutions sequentially instead of in parallel.       #
#                                                                          #
#  There are several places where KHE calls Unix time functions to         #
#  find out how much time has been consumed while solving.  You can        #
#  remove all these calls by finding line "#define KHE_USE_TIMING 1"       #
#  near the top of khe.h and changing the 1 to a 0.  This will cause       #
#  all times reported by KHE to be 0.                                      #
#                                                                          #
#  Mail jeff@it.usyd.edu.au if you have any problems.                      #
#                                                                          #
############################################################################

CC = gcc

CFLAGS = -g -std=c99 -pedantic -Wall -pthread -O3


############################################################################
#                                                                          #
#   Separate files                                                         #
#                                                                          #
#   Separate files are source files with their own header files that       #
#   could be extracted from KHE and used separately:                       #
#                                                                          #
#     File               Contents                                 Guide    #
#     -----------------------------------------------------------------    #
#     m.[hc]              Extensible arrays and symbol tables     App A    #
#     khe_lset.[hc]       Extensible bit vectors                  App B    #
#     khe_priqueue.[hc]   Updateable priority queues              App C    #
#     kml.[hc]            XML reading and writing                 App D    #
#     khe_wmatch.[hc]     Weighted bipartite matching               -      #
#     khe_partition.[hc]  Partitions (e.g. 2 1 is partition of 3)   -      #
#     khe_part_graph.[hc] Graph partitioning (for two-colouring)    -      #
#     -----------------------------------------------------------------    #
#                                                                          #
############################################################################

SEPARATE_OBJS = m.o kml.o khe_lset.o khe_wmatch.o khe_partition.o	\
  khe_priqueue.o khe_part_graph.o


############################################################################
#                                                                          #
#  Platform files                                                          #
#                                                                          #
#  Platform files implement KHE's platform.  They #include file            #
#  khe_interns.h, which includes khe.h and the header files of all the     #
#  separate modules except khe_partition.h and khe_priqueue.h.             #
#                                                                          #
#     Files              Contents                                 Guide    #
#     -----------------------------------------------------------------    #
#     INSTANCE_OBJS      Archives, soln groups, instances        Ch 2-3    #
#     CONSTRAINT_OBJS    Constraints                             Sc 3.7    #
#     SOLN_OBJS          Solns                                   Ch 4-5    #
#     MONITOR_OBJS       Monitors                                Ch 6-7    #
#     -----------------------------------------------------------------    #
#                                                                          #
############################################################################

PLATFORM_HDRS = khe_interns.h khe.h kml.h m.h khe_lset.h khe_matching.h	\
  khe_wmatch.h

INSTANCE_OBJS = khe_archive.o khe_archive_metadata.o khe_soln_group.o	\
  khe_soln_group_metadata.o khe_instance.o khe_instance_metadata.o	\
  khe_time_group_nhood.o khe_time_group.o khe_time.o			\
  khe_resource_type.o khe_resource_group.o khe_resource.o		\
  khe_event_group.o khe_event.o khe_event_resource.o			\
  khe_event_resource_group.o

CONSTRAINT_OBJS = khe_constraint.o					  \
  khe_assign_resource_constraint.o khe_assign_time_constraint.o		  \
  khe_split_events_constraint.o khe_distribute_split_events_constraint.o  \
  khe_prefer_resources_constraint.o khe_prefer_times_constraint.o	  \
  khe_avoid_split_assignments_constraint.o khe_spread_events_constraint.o \
  khe_link_events_constraint.o khe_order_events_constraint.o		  \
  khe_avoid_clashes_constraint.o khe_avoid_unavailable_times_constraint.o \
  khe_limit_idle_times_constraint.o khe_cluster_busy_times_constraint.o	  \
  khe_limit_busy_times_constraint.o khe_limit_workload_constraint.o

SOLN_OBJS = khe_soln.o khe_evenness_handler.o khe_meet.o		\
  khe_meet_bound.o khe_cycle_meet.o khe_task.o khe_task_bound.o		\
  khe_mark.o khe_path.o khe_trace.o khe_node.o khe_layer.o khe_zone.o	\
  khe_tasking.o khe_event_in_soln.o khe_event_resource_in_soln.o	\
  khe_resource_in_soln.o khe_matching.o

MONITOR_OBJS = khe_monitor.o						\
  khe_assign_resource_monitor.o khe_assign_time_monitor.o		\
  khe_split_events_monitor.o khe_distribute_split_events_monitor.o	\
  khe_prefer_resources_monitor.o khe_prefer_times_monitor.o		\
  khe_avoid_split_assignments_monitor.o khe_spread_events_monitor.o	\
  khe_link_events_monitor.o khe_order_events_monitor.o			\
  khe_avoid_clashes_monitor.o khe_avoid_unavailable_times_monitor.o	\
  khe_limit_idle_times_monitor.o khe_cluster_busy_times_monitor.o	\
  khe_limit_busy_times_monitor.o khe_limit_workload_monitor.o		\
  khe_timetable_monitor.o khe_time_group_monitor.o khe_group_monitor.o	\
  khe_ordinary_demand_monitor.o khe_workload_demand_monitor.o		\
  khe_evenness_monitor.o

PLATFORM_OBJS = $(INSTANCE_OBJS) $(CONSTRAINT_OBJS) $(SOLN_OBJS) $(MONITOR_OBJS)


############################################################################
#                                                                          #
#   Miscellaneous solver files                                             #
#                                                                          #
#   Miscellaneous solver files contain solvers and related functions,      #
#   mainly from Chapter 8, that do not fit into other solver categories.   #
#                                                                          #
#   File                        Contents                          Guide    #
#   ---------------------------------------------------------------------  #
#   khe_sm_workload.c           Workload demand nodes              7.4     #
#   khe_sm_general_solve.c      General solving                    8.1     #
#   khe_sm_parallel_solve.c     Parallel solving                   8.2     #
#   khe_sm_benchmark.c          Benchmarking                       8.3     #
#   khe_sm_options.c            Options                            8.4     #
#   khe_sm_stats.c              Statistics                         8.5     #
#   khe_sm_backoff.c            Exponential backoff                8.6     #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_MISC_HDRS = khe.h m.h

SOLVER_MISC_OBJS = khe_sm_workload.o khe_sm_general_solve.o		\
  khe_sm_parallel_solve.o khe_sm_benchmark.o khe_sm_options.o		\
  khe_sm_stats.o khe_sm_backoff.o


############################################################################
#                                                                          #
#   Layer tree construction solver files                                   #
#                                                                          #
#   Layer tree construction solver files contain the implementation of     #
#   the layer tree construction algorithm (Section 9.1).  Their names      #
#   have the form khe_sl_*.c.  Like all solver files they depend on        #
#   khe.h but not on khe_interns.h.  They also have their own private      #
#   header file, khe_sl_layer_tree.h.                                      #
#                                                                          #
############################################################################

SOLVER_LAYER_TREE_HDRS = khe_sl_layer_tree.h khe.h m.h khe_partition.h

SOLVER_LAYER_TREE_OBJS = khe_sl_layer_tree.o khe_sl_split_forest.o	\
  khe_sl_split_class.o khe_sl_split_layer.o khe_sl_split_job.o		\
  khe_sl_avoid_clashes_split_job.o khe_sl_pack_split_job.o		\
  khe_sl_preassigned_split_job.o khe_sl_assigned_split_job.o		\
  khe_sl_link_split_job.o khe_sl_split_split_job.o			\
  khe_sl_distribute_split_job.o khe_sl_spread_split_job.o		\
  khe_sl_domain_split_job.o


############################################################################
#                                                                          #
#   Structural solver files                                                #
#                                                                          #
#   Structural solver files contain structural solvers (Chapter 9),        #
#   excluding those that implement layer tree construction.                #
#                                                                          #
#   File                        Contents                          Guide    #
#   ---------------------------------------------------------------------  #
#   khe_ss_time_equiv.c         Time-equivalence                   9.2     #
#   khe_ss_layers.c             Layers                             9.3     #
#   khe_ss_runarounds.c         Runarounds                         9.4     #
#   khe_ss_nodes.c              Rearranging nodes                  9.6     #
#   khe_ss_zones.c              Zones                              9.6     #
#   khe_ss_splits.c             Analysing split defects            9.7.1   #
#   khe_ss_misc.c               Merging adjacent meets             9.7.2   #
#   khe_ss_grouping.c           Monitor attachment and grouping    9.8     #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_STRUCTURAL_HDRS = khe.h m.h

SOLVER_STRUCTURAL_OBJS = khe_ss_time_equiv.o khe_ss_layers.o		\
  khe_ss_runarounds.o khe_ss_nodes.o khe_ss_zones.o khe_ss_splits.o	\
  khe_ss_misc.o khe_ss_grouping.o


############################################################################
#                                                                          #
#   Time solver files                                                      #
#                                                                          #
#   Time solver files contain time solvers (Chapter 10).                   #
#                                                                          #
#   File                        Contents                          Guide    #
#   ---------------------------------------------------------------------  #
#   khe_st_helper.c             Helper functions                  10.2.1   #
#   khe_st_kempe.c              Kempe and ejecting meet moves     10.2.2   #
#   khe_st_meet_bound_group.c   Meet bound groups                 10.3.1   #
#   khe_st_domain.c             Domain reduction                  10.3.2   #
#   khe_st_cluster_and_limit.c  Domain reduction                  10.3.3   #
#   khe_st_basic.c              Basic time solvers                10.4     #
#   khe_st_runaround.c          Runarounds                        10.5     #
#   khe_st_elm_core.c           Elm - core module and main fn     10.6.2   #
#   khe_st_elm_split.c          Elm - splitting supplies          10.6.3   #
#   khe_st_elm_node.c           Elm - improving node regularity   10.6.4   #
#   khe_st_elm_irregular.c      Elm - handling irregular monitors 10.6.5   #
#   khe_st_layer_node_match.c   Layer node matching               10.7.1   #
#   khe_st_tree_repair.c        Tree search layer time repair     10.7.3   #
#   khe_st_meet_set_repair.c    Meet set time repair              10.7.4   #
#   khe_st_layer_asst.c         Layer assignments                 10.8.1   #
#   khe_st_layered.c            A solver for layered time asst    10.8.2   #
#   khe_st_combined.c           A complete time solver            10.8.3   #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_TIME_HDRS = khe.h m.h khe_wmatch.h khe_elm.h

SOLVER_TIME_OBJS =  khe_st_helper.o khe_st_kempe.o			\
  khe_st_meet_bound_group.o khe_st_domain.o khe_st_cluster_and_limit.o	\
  khe_st_basic.o khe_st_runaround.o khe_st_elm_core.o			\
  khe_st_elm_split.o khe_st_elm_node.o khe_st_elm_irregular.o		\
  khe_st_layer_node_match.o khe_st_tree_repair.o			\
  khe_st_meet_set_repair.o khe_st_layer_asst.o khe_st_layered.o		\
  khe_st_combined.o


############################################################################
#                                                                          #
#   Resource solver files                                                  #
#                                                                          #
#   Resource solver files contain resource solvers (Chapter 11).           #
#                                                                          #
#   File                        Contents                          Guide    #
#   ---------------------------------------------------------------------  #
#   khe_sr_invariant.c          Resource assignment invariant     11.2     #
#   khe_sr_task_bound_group.c   Task bound groups                 11.3.1   #
#   khe_sr_task_tree.c          Task trees                        11.3     #
#   khe_sr_task_group.c         Task groups                       11.3.5   #
#   khe_sr_first_resource.c     Most-constrained-first            11.4     #
#   khe_sr_pack_resource.c      Resource packing                  11.5     #
#   khe_sr_split_resource.c     Split assignments                 11.6     #
#   khe_sr_kempe.c              Kempe and ejecting task moves     11.7     #
#   khe_sr_resource_pair.c      Resource pair repair              11.9     #
#   khe_sr_rematch.c            Resource rematching               11.10    #
#   khe_sr_combined.c           Trying unassignments              11.11    #
#   khe_sr_combined.c           Putting it all together           11.12    #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_RESOURCE_HDRS = khe.h m.h khe_part_graph.h

SOLVER_RESOURCE_OBJS = khe_sr_invariant.o khe_sr_task_bound_group.o	\
  khe_sr_task_tree.o khe_sr_task_group.o khe_sr_first_resource.o	\
  khe_sr_pack_resource.o khe_sr_split_resource.o khe_sr_kempe.o		\
  khe_sr_resource_pair.o khe_sr_rematch.o khe_sr_combined.o


############################################################################
#                                                                          #
#  Ejection chain files                                                    #
#                                                                          #
#  Ejectors and the solvers that use them (Chapter 12).                    #
#                                                                          #
#   File                        Contents                          Guide    #
#   ---------------------------------------------------------------------  #
#   khe_se_ejector.c            Ejectors                          12.2-6   #
#   khe_se_solvers.c            Ejection chain solvers            12.7     #
#   khe_se_primary.c            Primary groupings for ejectors    12.7.1   #
#   khe_se_secondary.c          Secondary groupings for ejectors  12.7.2   #
#   ---------------------------------------------------------------------  #
#                                                                          #
############################################################################

SOLVER_EJECTION_HDRS = khe.h m.h

SOLVER_EJECTION_OBJS = khe_se_ejector.o khe_se_solvers.o		\
  khe_se_primary.o khe_se_secondary.o


############################################################################
#                                                                          #
#   Main rule, for producing binary "khe"                                  #
#                                                                          #
#   File khe_main.c contains a main() function, and invoking this rule     #
#   produces a binary which calls that function.  You can always throw     #
#   away this binary and khe_main.o and link the other .o files to your    #
#   own code.                                                              #
#                                                                          #
############################################################################

SOLVER_OBJS = $(SOLVER_MISC_OBJS) $(SOLVER_LAYER_TREE_OBJS)		\
  $(SOLVER_STRUCTURAL_OBJS) $(SOLVER_TIME_OBJS) $(SOLVER_RESOURCE_OBJS)	\
  $(SOLVER_EJECTION_OBJS)

khe : $(SEPARATE_OBJS) $(PLATFORM_OBJS) $(SOLVER_OBJS) khe_main.o
	$(CC) -o khe $(SEPARATE_OBJS) $(PLATFORM_OBJS) $(SOLVER_OBJS)	\
	  khe_main.o -lexpat -lm -pthread


############################################################################
#                                                                          #
#   Rules for separate files                                               #
#                                                                          #
#   Each of these can be compiled separately from the others and from      #
#   KHE generally, except that some use m.h.                               #
#                                                                          #
############################################################################

m.o : m.h
kml.o : kml.h m.h
khe_lset.o : khe_lset.h
khe_wmatch.o : khe_wmatch.h m.h
khe_partition.o : khe_partition.h m.h
khe_priqueue.o : khe_priqueue.h
khe_part_graph.o : khe_part_graph.h m.h


############################################################################
#                                                                          #
#   Rules for platform and solver files, and khe_main.o                    #
#                                                                          #
#   File khe_main.o is basically a solver, but it includes options for     #
#   testing various things, making it a bit different.                     #
#                                                                          #
############################################################################

$(PLATFORM_OBJS) : $(PLATFORM_HDRS)
$(SOLVER_LAYER_TREE_OBJS) : $(SOLVER_LAYER_TREE_HDRS)
$(SOLVER_STRUCTURAL_OBJS) : $(SOLVER_STRUCTURAL_HDRS)
$(SOLVER_TIME_OBJS) : $(SOLVER_TIME_HDRS)
$(SOLVER_RESOURCE_OBJS) : $(SOLVER_RESOURCE_HDRS)
$(SOLVER_EJECTION_OBJS) : $(SOLVER_EJECTION_HDRS)
$(SOLVER_MISC_OBJS) : $(SOLVER_MISC_HDRS)
khe_main.o : khe.h khe_partition.h khe_lset.h


############################################################################
#                                                                          #
#   Make clean (also cleans up doc subdirectory)                           #
#                                                                          #
############################################################################

clean:	
	-rm -f *.o khe
	-rm -fr doc/lout.li doc/*.ld doc/op*
