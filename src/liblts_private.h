  class lts;

  typedef struct
  {
    unsigned int from;
    unsigned int label;
    unsigned int to;
  } transition;

  class p_state_iterator
  {
    protected:
      lts *l;
      unsigned int pos;
      unsigned int max;
  };

  class p_label_iterator
  {
    protected:
      lts *l;
      unsigned int pos;
      unsigned int max;
  };

  class p_transition_iterator
  {
    protected:
      lts *l;
      unsigned int pos;
      unsigned int max;
  };

  class p_lts
  {
    protected:
      lts_type type;
      std::string creator;
      bool state_info;
      bool label_info;
      unsigned int states_size;
      unsigned int labels_size;
      unsigned int transitions_size;
      unsigned int nstates;
      unsigned int nlabels;
      unsigned int ntransitions;
      unsigned int *states;
      ATerm *state_values;
      unsigned int *labels;
      ATerm *label_values;
      transition *transitions;
      unsigned int init_state;

      lts_type detect_type(std::string &filename);
      lts_type detect_type(std::istream &is);

      void init();
      void clear();

      unsigned int p_add_state(ATerm value = NULL);
      unsigned int p_add_label(ATerm value = NULL);
      unsigned int p_add_transition(unsigned int from,
                                    unsigned int label,
                                    unsigned int to);

      bool read_from_svc(std::string &filename, lts_type type);
      bool read_from_aut(std::string &filename);
      bool read_from_aut(std::istream &is);
      bool write_to_svc(std::string &filename, lts_type type);
      bool write_to_aut(std::string &filename);
      bool write_to_aut(std::ostream &os);
  };
