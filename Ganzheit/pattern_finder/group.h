
#ifndef GROUP_H
#define GROUP_H


#include <opencv2/imgproc/imgproc.hpp>
#include <list>


namespace group {
    
    class Element {
        
    public:
        
        Element(cv::Point _p = cv::Point(), int _label = 0, int _index_in_list = 0);
        
        /*
         * Reset the current label to the initial position
         */
        void reset();

        /* The 2D coordinate of this element */
        cv::Point p;

        /* The current label in the pattern */
        int label;

        /**/
        int index_in_orig_list;
        
        /*
         * The label initially. When reset() is called, lable
         * will be set to label_init.
         */
        int label_init;

    };


    class Group {

    public:

        Group();

        Group(std::vector<Element> &l, int _id, int _start_index);

        /*
         * Set all possible configurations in this group. The number of configurations
         * is dependent on the number of members in the following way:
         *
         *   member count | number of configurations
         *   -----------------------------------------
         *    0           |  0
         *    1           |  3
         *    2           |  3
         *    3           |  1
         *
         */
        void initialiseGroupConfigurations();

        bool nextConfiguration();
        bool prevConfiguration();
        void initConfiguration();

        const std::vector<Element> 				&getMembers() const		{return members;}
        std::vector<Element>					&getMembers()			{return members;}

        int getID() const {return id;}

        size_t size() const {return members.size();}

    private:

        /* The vector of members */
        std::vector<Element> members;

        /* Group id, not really needed for other than debuggin purposes. */
        int id;

        /*
         * The start label of this group:
         *   Group1: 0
         *   Group2: 1
         *   Group3: 3
         *   Group4: 4
         *
         * Stays constant after the constructor
         */
        int startLabel;

        // current configuration
        int config;


        /*
         * The labels for all memebers in all configurations.
         * For example in case there are 2 members in this group,
         * the positions are:
         *
         *    // member 1
         *    positions[0][0] = startLabel
         *    positions[0][1] = startLabel
         *    positions[0][2] = startLabel + 1
         *
         *    // member 2
         *    positions[1][0] = startLabel + 1
         *    positions[1][1] = startLabel + 2
         *    positions[1][2] = startLabel + 2
         *
         */
        std::vector<std::vector<int> > positions;

    private:

        // number of different configurations
        int nof_config;

    };



    class Configuration {

    public:

        Configuration() {}

        Configuration(double  _err) {err = _err;}

        std::vector<Element> elements;

        double err;

    };


    class GroupManager {
    public:

        bool identify(std::vector<cv::Point> &extracted, cv::Point pupil_centre);

        const std::vector<Group> &getGroups() const {return groups;}
        std::vector<Group> &getGroups() {return groups;}

        const std::vector<Configuration> &getConfigurations() const {return configurations;}

        const Configuration &getBestConfiguration() {return config_best;}

    private:

        /*
         * Tests if the elements occupy vacant labels or not.
         */
        static bool testVacancy(const int table[6], const std::vector<Element> &elements);

        /*
         * Occupies the labels defined in elemnts.
         */
        static void addToVacancyTable(const std::vector<Element> &elements, int table[6]);

        /*
         * Assigns the extracted point into the correct groups.
         */
        bool assignToGroups(std::vector<cv::Point> &extracted, cv::Point pupil_centre);

        /*
         * Calls Group::initConfiguration() for all groups
         */
        void initialiseGroups();

        /*
         * Called after the elements have been assigned to their groups.
         * Sorts the elements according their 2D coordinates.
         */
        void sortGroups();

        /*
         * Compute the error of the current configuration.
         */
        double computeError();

        /*
         * Computes the current configuration's error and places
         * the configuration in the list. This function will be
         * called only for valid configurations detected in
         * loopConfigurations().
         */
        void storeConfiguration();

        /*
         * The main looping. Loops through all configurations and
         * selects all valid ones. This is a recursive function.
         */
        void loopConfigurations(std::vector<Group>::iterator git);

        /*
         * Tests the current configurations validity. Two
         * checks are performed in the check:
         *
         * 1. The last member of a group can not exceed
         *    the first member in the next group, in terms
         *    of label in the pattern.
         *
         * 2. There must be no overlap in the configurations,
         *    i.e. all labels in the pattern can be occupied
         *    only once.
         */
        bool isConfigOk();

        /* Vector of all groups, will always be resized to 4. */
        std::vector<Group> groups;

        /* A list of valid configurations. */
        std::vector<Configuration> configurations;

        /* The best configuration selected from the list. */
        Configuration config_best;

        /* The number of points given in identify() */
        int nof_points;

    };


} // end of "namespace group {"


#endif
