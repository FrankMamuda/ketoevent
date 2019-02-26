/*
 * Copyright (C) 2019 Factory #12
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

// event parameters (TODO: must be configured externally)
var EventParms = {
    ComboOfTwo:      1,
    ComboOfThree:    3,
    ComboOfFourPlus: 5,
    StartTime:       JS.timeFromString( '11:00' ),
    FinishTime:      JS.timeFromString( '17:00' ),
    FinalTime:       JS.timeFromString( '17:30' ),
    Penalty:         5
};


/**
 * @brief Structure
 */
function Structure( teamTitle ) {
    // "enum" of column ids
    this.Rank = 0;
    this.Title = 1;
    this.Completed = 2;
    this.Combos = 3;
    this.ComboTasks = 4;
    this.Penalty = 5;
    this.Time = 6;
    this.Points = 7;
    this.Count = 8;

    // column properties such as name and value
    this.properties = {
        0: { name: 'rank',           displayName: 'Rank',             value: 1 },
        1: { name: 'title',          displayName: 'Title',            value: '' },
        2: { name: 'completedTasks', displayName: 'Completed\ntasks', value: 0 },
        3: { name: 'combos',         displayName: 'Combinations',     value: 0 },
        4: { name: 'comboTasks',     displayName: 'Combined\ntasks',  value: 0 },
        5: { name: 'penalty',        displayName: 'Penalty\npoints',  value: 0 },
        6: { name: 'time',           displayName: 'Time on\ntrack',   value: 0 },
        7: { name: 'points',         displayName: 'Total\npoints',    value: 0 },
    };

    // setters
    this.set = function( prop, v ) { this.properties[prop].value = v; };
    this.setRank           = function( v ) { this.set( this.Rank,       v ); };
    this.setTitle          = function( v ) { this.set( this.Title,      v ); };
    this.setCompletedTasks = function( v ) { this.set( this.Completed,  v ); };
    this.setCombos         = function( v ) { this.set( this.Combos,     v ); };
    this.setComboTasks     = function( v ) { this.set( this.ComboTasks, v ); };
    this.setPenalty        = function( v ) { this.set( this.Penalty,    v ); };
    this.setTime           = function( v ) { this.set( this.Time,       v ); };
    this.setPoints         = function( v ) { this.set( this.Points,     v ); };

    // getters
    this.value = function( prop ) { return this.properties[prop].value; };
    this.rank           = function( prop ) { return this.value( this.Rank       ); };
    this.title          = function( prop ) { return this.value( this.Title      ); };
    this.completedTasks = function( prop ) { return this.value( this.Completed  ); };
    this.combos         = function( prop ) { return this.value( this.Combos     ); };
    this.comboTasks     = function( prop ) { return this.value( this.ComboTasks ); };
    this.penalty        = function( prop ) { return this.value( this.Penalty    ); };
    this.time           = function( prop ) { return this.value( this.Time       ); };
    this.points         = function( prop ) { return this.value( this.Points     ); };

    /**
     * @brief data exports QList<QVariantList> of team statistics
     */
    this.data = function( t ) {
        var out = [];
        for ( var y = 0; y < this.Count; y++ )
            out.push( this.properties[y].value );
        return out;
    }

    /**
     * @brief columns exports column display names
     */
    this.columns = function( t ) {
        var out = [];
        for ( var y = 0; y < this.Count; y++ )
            out.push( this.properties[y].displayName );
        return out;
    }

    // initialize team title
    this.setTitle( teamTitle );
}

// structure template (for exports)
var defaultStructure = new Structure();

/**
 * @brief main
 */
function main() {
    // announce
    print( 'Hello world' );
}

/**
 * @brief data
 */
function data() {
    // test if any events are active
    var event = Main.currentEvent();
    if ( event === -1 ) {
        print( 'No active event' )
        return;
    } else {
        print( 'Event', Event.title( event ))
    }

    // stats list
    var stats = [];

    // go through team list
    for ( var team = 0; team < Team.count(); team++ ) {
        // TODO: validate row by id?

        // initialize stats
        var teamStats = new Structure( Team.title( team ));

        // create comboMap
        var combos = {};
        var dup = [];

        // go through logs
        for ( var log = 0; log < Log.count(); log++ ) {
            // TODO: validate row by id?
            var value = Log.multiplier( log );

            // abort on invalid log values or logs not related to the current team
            if ( value <= 0 || Team.id( team ) !== Log.teamId( log ))
                continue;

            // get task id from log
            var taskId = Log.taskId( log );
            if ( taskId === -1 ) {
                print( 'bad task id' );
                continue;
            }

            // get task row from task id
            var task = Task.row( taskId );
            if ( task === -1 ) {
                print( 'bad task row' );
                continue;
            }

            // get task type
            var type = Task.type( task );

            // test for duplicates
            if ( dup.indexOf( taskId ) !== -1 )
                continue;

            dup.push( taskId );

            // increment completed tasks
            teamStats.setCompletedTasks( teamStats.completedTasks() + 1 );

            // calculate points form completed tasks
            teamStats.setPoints( teamStats.points() + Task.points( task ) * (( type === 1 ) ? value : 1 ));

            // build combo map
            var comboId = Log.comboId( log );
            if ( !combos.hasOwnProperty( comboId ) && comboId > -1 )
                combos[comboId] = 0;

            if ( comboId > -1 ) {
                combos[comboId] = combos[comboId] + 1;
                teamStats.setComboTasks( teamStats.comboTasks() + 1 );
            }
        }

        // store combo count
        teamStats.setCombos( Object.keys( combos ).length );

        // calculate bonus points from combos
        for ( var key in combos ) {
            if ( combos.hasOwnProperty( key )) {
                count = combos[key];

                if ( count === 2 )
                    teamStats.setPoints( teamStats.points() + EventParms.ComboOfTwo );

                if ( count === 3 )
                    teamStats.setPoints( teamStats.points() + EventParms.ComboOfThree );

                if ( count >= 4 )
                    teamStats.setPoints( teamStats.points() + EventParms.ComboOfFourPlus );
            }
        }

        // calculate penalty points
        var teamFinishTime =  Team.finishTime( team );
        var overTime = ( EventParms.FinishTime - teamFinishTime ) / 60000 - 1;
        teamStats.setTime( Math.max( 0, ( teamFinishTime - EventParms.StartTime ) / 60000 + 1 ));

        if ( overTime < 0 ) {
            teamStats.setPenalty( EventParms.Penalty * Math.abs( overTime ));
            teamStats.setPoints( teamStats.points() - teamStats.penalty());
            teamStats.setPoints( Math.max( teamStats.points(), 0 ));
        }

        if ( teamFinishTime - EventParms.FinalTime > 0 )
            teamStats.setPoints( 0 );

        // announce results
        //print( teamStats.title(), "points:", teamStats.points(), "combos:",
        //      teamStats.combos(), "penalty:", teamStats.penalty(), "time:", teamStats.time());

        // add to stats list (list of QVariantMap)
        stats.push( teamStats );
    }

    // sort by points
    stats.sort( function( a, b ) {  return b.points() - a.points(); } );

    // assign rank and build export list
    var rank = 0;
    var last = 0;
    var out = [];
    for ( var y = 0; y < stats.length; y++ ) {
        var points = stats[y].points();
        if ( last !== points )
            rank++;

        stats[y].setRank( rank );
        last = points;

        // add data to list
        out.push( stats[y].data());
    }

    // export list
    return out;
}

/**
 * @brief rankingsColumns exports column names for rankings
 */
function columns() {
    return defaultStructure.columns();
}

/**
 * @brief options returns a list of configurable options
 */
function options() {
    // name, type, default value
    return [ 'Combo of Two;int;1', 'Finish time;QTime;15:50' ];
}

// TODO: export formating such as font, colours, etc.
